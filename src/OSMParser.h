#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdint>
#include <zlib.h>

#include "Proto/fileformat.pb.h"
#include "Proto/osmformat.pb.h"
#include "OSMDispatcher.h"


class OSMParser{
    public:
        OSMParser();
        ~OSMParser();
    private:
        uint32_t getBlobLenght(std::vector<char> &data);
         

        std::ifstream m_file;
        uint64_t fileIndex;
        uint64_t m_fileSize;
        FV::OSMDispatcher m_dispatcher;


};