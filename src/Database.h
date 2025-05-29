#pragma once

#include <mutex>
#include <vector>

#include <sqlite3.h>

#include "Proto/ProtoStructs.h"

namespace FV{
    class Database{
        public:
            Database(std::string dbPath);
            ~Database();

            void saveNodes(std::vector<FV::Node> &nodes);
            void saveKeyValues(std::vector<FV::KeyValue> &keyValues);
            void saveWays(std::vector<int64_t> &wayIDS);
            void saveWayRefs(std::vector<FV::NodeRef> &refs);         
            void saveRelations(std::vector<int64_t> &relationIDS);
            void saveRelationRoles(std::vector<FV::RelationRole> &relationRoles);   

            bool skipKey(std::string &key);

            uint64_t getKeyIndex(const std::string &key);

        private:
            void initializeDB();
            void createSkipKeys();

            std::string m_dbPath;
            sqlite3 *m_db;
            int m_openResult;

            std::mutex m_dbMutex;
            std::vector<std::string> m_skipKeys;

        
    };
}