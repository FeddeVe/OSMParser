#include "OSMDispatcher.h"
#include "utils.h"

FV::OSMDispatcher::OSMDispatcher()
{
}

FV::OSMDispatcher::~OSMDispatcher()
{
    m_file.close();
}

void FV::OSMDispatcher::start(std::string osmFIle)
{
    m_progressFile = FV::getFileNameWithoutType(osmFIle) + ".json";
    m_sqlFile = FV::getFileNameWithoutType(osmFIle) + ".db";
    if (!FV::fileExists(m_progressFile))
    {
        m_progressFile = createProgressFile(osmFIle);
    };

    std::ifstream ifs(m_progressFile);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document d;
    d.ParseStream(isw);
    if (d.HasParseError())
    {
        std::cerr << "Error parsing JSON file: " << m_progressFile << std::endl;
        return;
    }

    if (d.HasMember("Progress"))
    {
        m_byteIndex = d["Progress"].GetUint64();
    }
    else
    {
        std::cerr << "No Progress member found in JSON file: " << m_progressFile << std::endl;
        return;
    }
    if (d.HasMember("OSMFile"))
    {
        m_osmFile = d["OSMFile"].GetString();
        m_file.open(m_osmFile.c_str(), std::ios::in | std::ios::binary);
        if (!m_file.is_open())
        {
            assert("Could not open file");
        }
        m_file.seekg(0, std::ios::end);
        m_fileSize = m_file.tellg();
    }
    else
    {
        std::cerr << "No OSMFile member found in JSON file: " << m_progressFile << std::endl;
        return;
    }
    if (d.HasMember("SQLFile"))
    {
        m_sqlFile = d["SQLFile"].GetString();
        if (!FV::fileExists(m_sqlFile))
        {
            //  std::cerr << "SQL file does not exist: " << m_sqlFile << std::endl;
            //  return;
        }
    }

    m_db = std::make_shared<FV::Database>(m_sqlFile);
    doWork();
}

std::string FV::OSMDispatcher::createProgressFile(std::string OSMFile)
{
  //  std::string filePath = FV::getDirectoryFromPath(OSMFile);
  //  std::string sqlPath = FV::getDirectoryFromPath(OSMFile);
    //std::string progressPath = filePath + "/progress.json";

    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value osmFile;
    osmFile.SetString(OSMFile.c_str(), OSMFile.length(), doc.GetAllocator());
    doc.AddMember("OSMFile", osmFile, doc.GetAllocator());
    rapidjson::Value sqlFile;
    sqlFile.SetString(m_sqlFile.c_str(), m_sqlFile.length(), doc.GetAllocator());
    doc.AddMember("SQLFile", sqlFile, doc.GetAllocator());
    doc.AddMember("Progress", 0, doc.GetAllocator());

    FILE *fp = fopen(m_progressFile.c_str(), "wb"); // non-Windows use "w"

    char writeBuffer[65536];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

    rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);

    fclose(fp);

    return m_progressFile;
}

uint32_t FV::OSMDispatcher::getBlobLenght(std::vector<char> &data)
{
    uint32_t ret = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    return ret;
}

void FV::OSMDispatcher::doWork()
{
    size_t numThreads = std::thread::hardware_concurrency();
    //numThreads = 1; // for testing purposes, use only one thread
    m_workers.resize(numThreads);
    m_futures.resize(numThreads - 1); // last thread is this thread
    m_startTime = time(0);
    m_startByte = m_byteIndex;

    if (!m_file.is_open())
    {
        assert("File not open");
    }

    displayProgress();

    for (size_t i = 0; i < m_workers.size() - 1; i++)
    {
        dispatch(&m_workers[i]);
        m_futures[i] = m_threadPool.addQuee(std::bind(&FV::OSMWorker::run, &m_workers[i]));
    }
    dispatch(&m_workers[m_workers.size() - 1]);
    m_workers[m_workers.size() - 1].run(); // run the last worker in this thread

    bool isDone = false;
    while (!isDone)
    {
        // waiting for all threads to finish
        for (size_t i = 0; i < m_futures.size(); i++)
        {
            m_futures[i].get();
        }
        displayProgress();
        saveProgress();
        if(m_byteIndex >= m_fileSize){
            isDone = true;
            continue;
        }
        for (size_t i = 0; i < m_workers.size() - 1; i++)
        {
            dispatch(&m_workers[i]);
            m_futures[i] = m_threadPool.addQuee(std::bind(&FV::OSMWorker::run, &m_workers[i]));
        }
        dispatch(&m_workers[m_workers.size() - 1]);
        m_workers[m_workers.size() - 1].run(); // run the last worker in this thread
    }
}

void FV::OSMDispatcher::dispatch(FV::OSMWorker *worker)
{
    m_file.seekg(m_byteIndex, std::ios::beg);
    std::vector<char> buffer;
    buffer.resize(4);
    m_file.read(buffer.data(), 4);
    m_byteIndex += 4;
    uint32_t len = getBlobLenght(buffer);
    if (len > C_MAXBLOBHEADERSIZE)
    {
        std::cerr << "Blob header size is too large: " << len << std::endl;
        return;
    }
    buffer.resize(len);
    m_byteIndex += len;
    m_file.read(buffer.data(), len);
    OSMPBF::BlobHeader hdr;
    hdr.ParseFromArray(buffer.data(), buffer.size());
    uint32_t dataSize = hdr.datasize();
    if (dataSize > C_MAXBLOBSIZE)
    {
        std::cerr << "Blob data size is too large: " << dataSize << std::endl;
        return;
    }
    buffer.resize(dataSize);
    m_file.read(buffer.data(), dataSize);
    m_byteIndex += dataSize;
    worker->init(buffer, m_db, hdr);
}

void FV::OSMDispatcher::saveProgress()
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value osmFile;
    osmFile.SetString(m_osmFile.c_str(), m_osmFile.length(), doc.GetAllocator());
    doc.AddMember("OSMFile", osmFile, doc.GetAllocator());
    rapidjson::Value sqlFile;
    sqlFile.SetString(m_sqlFile.c_str(), m_sqlFile.length(), doc.GetAllocator());
    doc.AddMember("SQLFile", sqlFile, doc.GetAllocator());
    doc.AddMember("Progress", m_byteIndex, doc.GetAllocator());

    FILE *fp = fopen(m_progressFile.c_str(), "wb"); // non-Windows use "w"

    char writeBuffer[65536];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

    rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);

    fclose(fp);
}

void FV::OSMDispatcher::displayProgress()
{
    uint64_t currentTime = time(0);
    uint64_t elapsedTime = currentTime - m_startTime;

    uint64_t elapsedBytes = m_byteIndex - m_startByte;
    double percent = static_cast<double>(m_byteIndex) / (m_fileSize - m_startByte);
    uint64_t totalTime = elapsedTime / percent;
    uint64_t timetogo = totalTime - elapsedTime;
    std::cout << "Progress: " << percent * 100.0 << "% - Running Time: " << FV::timeToString(elapsedTime) << " - Time to go: " << FV::timeToString(timetogo) << std::endl;
}
