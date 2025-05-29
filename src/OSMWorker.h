#pragma once

#include <vector>
#include <memory>

#include <zlib.h>

#include "Database.h"
#include "Proto/fileformat.pb.h"
#include "Proto/osmformat.pb.h"
#include "Proto/ProtoStructs.h"

namespace FV{
    struct DenseNodeKeyValue{
        int64_t id;
        std::string key;
        std::string value;
    };

    class OSMWorker{
        public:
            OSMWorker();
            ~OSMWorker();

            void init(std::vector<char>Data, std::shared_ptr<FV::Database> dataBase, OSMPBF::BlobHeader hdr);
            void run();

        private:
            void processHeaderBlock();
            void processPrimitiveBlock(std::vector<char> &data);

            void processDenseNodes(const OSMPBF::DenseNodes &denseNodes, int32_t granularity, int64_t latOffset, int64_t lonOffset, OSMPBF::StringTable &stringTable);
            void processWay(const OSMPBF::PrimitiveGroup &primitiveGroup, OSMPBF::StringTable &stringTable);
            void processRelations(const OSMPBF::PrimitiveGroup &primitiveGroup, OSMPBF::StringTable &stringTable);

            double getLatLon(int64_t val, int32_t granularity, int64_t valOffset) {
                return (static_cast<double>(0.000000001) * (static_cast<double>(valOffset) + (static_cast<double>(granularity) * static_cast<double>(val))));
            };

           

            std::shared_ptr<FV::Database> m_db;
            OSMPBF::BlobHeader m_hdr;
            std::vector<char> m_data;

    };
}