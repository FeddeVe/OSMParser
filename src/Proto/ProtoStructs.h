#pragma once

#include <cstdint>
#include <string>

namespace FV{
    struct Node{
        uint64_t id;
        double lat;
        double lon;

        Node(uint64_t id, double lat, double lon)
            : id(id), lat(lat), lon(lon) {}
    };

    struct KeyValue{       
        std::string key;
        std::string value;
       uint64_t nodeID = 0;
       uint64_t wayID = 0;
       uint64_t relationID = 0;
        

    
    };

    struct NodeRef{
        uint64_t wayID;
        uint64_t nodeID;
        uint32_t refIndex;
        NodeRef(uint64_t wayID, uint64_t nodeID, uint32_t refIndex)
            : wayID(wayID), nodeID(nodeID), refIndex(refIndex) {}
    };

    struct RelationRole{
        uint64_t relationID;
        std::string role;
        int64_t memberID;
        int64_t memberType;
        RelationRole(uint64_t relationID, const std::string &role, int64_t memberID, int64_t memberType)
            : relationID(relationID), role(role), memberID(memberID), memberType(memberType) {}
        
    };
}