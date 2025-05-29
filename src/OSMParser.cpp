#include "OSMParser.h"

OSMParser::OSMParser()
{
    // GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::string filename = "C:/Users/Fedde/Downloads/bremen-latest.osm.pbf";
    //std::string progressFile = FV::OSMDispatcher::createProgressFile(filename);
    m_dispatcher.start(filename);


    m_file.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    if (!m_file.is_open())
    {
        assert("Could not open file");
    }

    m_fileSize = m_file.tellg();

    std::vector<char> buffer;
    buffer.resize(4);
    size_t index = 0;
    m_file.seekg(index, m_file.beg);
    m_file.read(buffer.data(), 4);
    index += 4;

    uint32_t len = getBlobLenght(buffer);
    buffer.resize(len);
    m_file.read(buffer.data(), len);
    OSMPBF::BlobHeader hdr;
    hdr.ParseFromArray(buffer.data(), buffer.size());
    std::string type = hdr.type();
    std::string data = hdr.indexdata();
    int32_t datasize = hdr.datasize();

    buffer.resize(datasize);
    m_file.read(buffer.data(), datasize);
    OSMPBF::Blob blob;
    blob.ParseFromArray(buffer.data(), buffer.size());
    int32_t rawSize = blob.raw_size();
    OSMPBF::Blob::DataCase cs = blob.data_case();
    if (cs == OSMPBF::Blob::DataCase::kZlibData)
    {
        std::string zlibRaw = blob.zlib_data();
        int zlibLen = zlibRaw.length();
        std::vector<char> output(rawSize);

        z_stream infstream;
        infstream.zalloc = Z_NULL;
        infstream.zfree = Z_NULL;
        infstream.opaque = Z_NULL;
        // setup "b" as the input and "c" as the compressed output
        infstream.avail_in = (uInt)zlibRaw.length();  // size of input
        infstream.next_in = (Bytef *)zlibRaw.c_str(); // input char array
        infstream.avail_out = (uInt)rawSize;          // size of output
        infstream.next_out = (Bytef *)output.data();  // output char array

        // the actual DE-compression work.
        inflateInit(&infstream);
        inflate(&infstream, Z_NO_FLUSH);
        inflateEnd(&infstream);

        printf("Uncompressed size is: %lu\n", output.size());
        printf("Uncompressed string is: %s\n", output.data());
          OSMPBF::HeaderBlock hdrBlock;
        hdrBlock.ParseFromArray(output.data(), output.size());
        OSMPBF::HeaderBBox bbox = hdrBlock.bbox(); 
        if(hdrBlock.has_bbox()){
            int bp = 0;
            bp++;
        }
        google::protobuf::RepeatedPtrField<std::string> req_features = hdrBlock.required_features();
         google::protobuf::RepeatedPtrField<std::string> opt_features = hdrBlock.optional_features();
         for(size_t i = 0; i < req_features.size(); i++){
            std::cout <<req_features[i] << std::endl;
         }
         
         std::cout << "---OPT FEATURES --- "<<std::endl;
         for(size_t i = 0; i < opt_features.size(); i++){
            std::cout <<opt_features[i] << std::endl;
         }

        int bp = 0;
        bp++;
    }

  
    int bp = 0;
    bp++;

    // OSMPBF::BlobHeader hdr;
    // hdr.ParseFromArray(rawData.data(), 4);
}

OSMParser::~OSMParser() {}

uint32_t OSMParser::getBlobLenght(std::vector<char> &data)
{
    uint32_t ret = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    return ret;
}
