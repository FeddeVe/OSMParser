#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "rapidjson/filewritestream.h"
#include <rapidjson/istreamwrapper.h>
#include <iostream>

#include <vector>
#include <time.h>
#include <cassert> 

#include "Proto/fileformat.pb.h"
#include "Proto/osmformat.pb.h"
#include "ThreadPool.h"
#include "OSMWorker.h"
#include "Database.h"



namespace FV{
    class OSMDispatcher{
        const uint64_t C_MAXBLOBHEADERSIZE = 32*1024; //32KB
        const uint64_t C_MAXBLOBSIZE = 32*1024*1024; //32MB

        public:
            OSMDispatcher();
            ~OSMDispatcher();

           void start(std::string osmFile);
           std::string createProgressFile(std::string filename);

        private:
            uint32_t getBlobLenght(std::vector<char> &data);
            void doWork();
            void dispatch(FV::OSMWorker *worker);
            void saveProgress();
            void createProgress();     
            void displayProgress();        

            uint64_t m_byteIndex;
            std::string m_osmFile;
            std::string m_sqlFile;
            std::ifstream m_file;
            uint64_t m_fileSize;
            uint64_t m_startTime;
            uint64_t m_startByte;
            std::mutex m_fileMutex;
            std::string m_progressFile;
            std::vector<FV::OSMWorker> m_workers; 
            std::vector<std::shared_future<void>> m_futures;
            std::shared_ptr<FV::Database> m_db;
            FVEngine::ThreadPool m_threadPool;
    };
}