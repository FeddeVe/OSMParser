#include "OSMWorker.h"

FV::OSMWorker::OSMWorker()
{
}

FV::OSMWorker::~OSMWorker()
{
}

void FV::OSMWorker::init(std::vector<char> Data, std::shared_ptr<FV::Database> dataBase, OSMPBF::BlobHeader hdr)
{
    m_data = std::move(Data);
    m_db = dataBase;
    m_hdr = hdr;
}

void FV::OSMWorker::run()
{
    OSMPBF::Blob blb;
    blb.ParseFromArray(m_data.data(), m_data.size());
    int32_t rawSize = blb.raw_size();
    OSMPBF::Blob::DataCase cs = blb.data_case();
    std::vector<char> data;
    switch (cs)
    {
    case OSMPBF::Blob::DataCase::kRaw:
        data = std::vector<char>(blb.raw().data(), blb.raw().data() + rawSize);
        break;
    case OSMPBF::Blob::DataCase::kZlibData:
        // Handle zlib data
        {
            std::string zlibRaw = blb.zlib_data();
            int zlibLen = zlibRaw.length();
            data.resize(rawSize);

            z_stream infstream;
            infstream.zalloc = Z_NULL;
            infstream.zfree = Z_NULL;
            infstream.opaque = Z_NULL;
            infstream.avail_in = (uInt)zlibRaw.length();  // size of input
            infstream.next_in = (Bytef *)zlibRaw.c_str(); // input char array
            infstream.avail_out = (uInt)rawSize;          // size of output
            infstream.next_out = (Bytef *)data.data();    // output char array

            // the actual DE-compression work.
            inflateInit(&infstream);
            inflate(&infstream, Z_NO_FLUSH);
            inflateEnd(&infstream);
        }
        break;
    case OSMPBF::Blob::DataCase::kLzmaData:
        std::cout << "LZMA data is not supported yet" << std::endl;
        break;
    case OSMPBF::Blob::DataCase::kOBSOLETEBzip2Data:
        std::cout << "Bzip2 data is not supported yet" << std::endl;
        break;
    case OSMPBF::Blob::DataCase::kLz4Data:
        std::cout << "LZ4 data is not supported yet" << std::endl;
        break;
    case OSMPBF::Blob::DataCase::kZstdData:
        std::cout << "Zstd data is not supported yet" << std::endl;
        break;
    default:
        std::cerr << "Unknown Blob data type" << std::endl;
        return;
    }

    if (m_hdr.type() == "OSMHeader")
    {
        // Process OSMHeader
        processHeaderBlock();
    }
    else if (m_hdr.type() == "OSMData")
    {
        // Process OSMData
        processPrimitiveBlock(data);
    }
    else
    {
        // Handle other types or errors
        std::cerr << "Unknown BlobHeader type: " << m_hdr.type() << std::endl;
    }
}

void FV::OSMWorker::processHeaderBlock()
{
    OSMPBF::HeaderBlock hdrBlock;
    hdrBlock.ParseFromArray(m_data.data(), m_data.size());
}

void FV::OSMWorker::processPrimitiveBlock(std::vector<char> &data)
{
    OSMPBF::PrimitiveBlock primBlock;
    primBlock.ParseFromArray(data.data(), data.size());
    OSMPBF::StringTable stringTable = primBlock.stringtable();
    google::protobuf::RepeatedPtrField<OSMPBF::PrimitiveGroup> primGroup = primBlock.primitivegroup();
    int32_t granularity = primBlock.granularity();
    int64_t latOffset = primBlock.lat_offset();
    int64_t lonOffset = primBlock.lon_offset();
    int32_t dateGranularity = primBlock.date_granularity();

    for (size_t i = 0; i < primGroup.size(); i++)
    {
        const OSMPBF::PrimitiveGroup &group = primGroup[i];
        for (size_t j = 0; j < group.nodes_size(); j++)
        {
            const OSMPBF::Node &node = group.nodes(j);
            std::cout << "Node " << j << ": id=" << node.id()
                      << ", lat=" << (node.lat() * granularity + latOffset) / 1e7
                      << ", lon=" << (node.lon() * granularity + lonOffset) / 1e7
                      << std::endl;
        }
        if (group.has_dense())
        {
            const OSMPBF::DenseNodes &denseNodes = group.dense();
            processDenseNodes(denseNodes, granularity, latOffset, lonOffset, stringTable);
        }

        processWay(group, stringTable);
        processRelations(group, stringTable);

        for (size_t j = 0; j < group.changesets_size(); j++)
        {
            const OSMPBF::ChangeSet &changeset = group.changesets(j);
            std::cout << "Changeset " << j << ": id=" << changeset.id()
                      /*
                                << ", created_at=" << changeset.created_at()
                                << ", closed_at=" << changeset.closed_at()
                                */
                      << std::endl;
        }
        std::cout << "PrimitiveGroup " << i << " has " << group.nodes_size() << " nodes, "
                  << group.ways_size() << " ways, and " << group.relations_size() << " relations." << std::endl;
        if (group.has_dense())
        {
            std::cout << "And " << group.dense().id_size() << " dense nodes." << std::endl;
        }
    }

    int bp = 0;
    bp++;
}

void FV::OSMWorker::processDenseNodes(const OSMPBF::DenseNodes &denseNodes, int32_t granularity, int64_t latOffset, int64_t lonOffset, OSMPBF::StringTable &stringTable)
{
    int64_t oldID = 0;
    int64_t oldLat = 0;
    int64_t oldLon = 0;
    std::vector<FV::Node> nodes;
    std::vector<FV::KeyValue> keyValues;
    std::vector<FV::DenseNodeKeyValue> denseKeyNodes; 

    int64_t nodeIndex = 0;

    for(int i = 0; i < denseNodes.keys_vals_size(); i++){
        int32_t key = denseNodes.keys_vals(i);
        if(key == 0){
            nodeIndex++;
            continue;
        } 
        std::string keyStr = stringTable.s(key);
        i++;
        int32_t value = denseNodes.keys_vals(i);
        std::string valueStr = stringTable.s(value);
        FV::DenseNodeKeyValue kv;
        kv.id = nodeIndex;
        kv.key = keyStr;
        kv.value = valueStr;
        denseKeyNodes.push_back(kv);
    }
    

    int64_t oldKeyIndex = 0;
    for (int i = 0; i < denseNodes.id_size(); i++)
    {
        oldID += denseNodes.id(i);
        oldLat += denseNodes.lat(i);
        oldLon += denseNodes.lon(i);
        int64_t id = oldID;
        double lat = getLatLon(oldLat, granularity, latOffset);
        double lon = getLatLon(oldLon, granularity, lonOffset);

        bool visible = true;
        if (denseNodes.has_denseinfo())
        {
            const OSMPBF::DenseInfo &denseInfo = denseNodes.denseinfo();
            if (denseInfo.visible_size() > i)
            {
                visible = denseInfo.visible(i);
            }
            // visible = denseInfo.visible(i);
            // id = denseNodes.denseinfo().uid(i);
            // Process dense info if needed
        }

        for(size_t j = 0; j < denseKeyNodes.size(); j++){
            if(denseKeyNodes[j].id > i){
                break;
            }
            if(denseKeyNodes[j].id == i){ 
                    if(!m_db->skipKey(denseKeyNodes[j].key)){
                        // Create a new KeyValue object if it doesn't exist
                        FV::KeyValue kv{};
                        kv.key = denseKeyNodes[j].key;
                        kv.value = denseKeyNodes[j].value;
                        kv.nodeID = id;
                        kv.wayID = 0;
                        kv.relationID = 0;
                        keyValues.push_back(kv);
                    }
                
            }
        }

        /*
        size_t tagIndex = 0; 

        for (size_t j = 0; j < denseNodes.keys_vals_size(); j++)
        {             
            int32_t key = denseNodes.keys_vals(j);
            std::string keyStr = stringTable.s(key);
            if (keyStr == "")
            {
                tagIndex++;             
                continue;
            }
            j++;
            int32_t value = denseNodes.keys_vals(j);

            if (tagIndex == i)
            {
                std::string valueStr = stringTable.s(value);
                bool nieuw = true;
                for(size_t z = 0; z < keyValues.size(); z++){
                    if(keyValues[z].addNode(id, keyStr, valueStr)){
                        nieuw = false;
                        break;
                    }
                }
                if(nieuw){
                    if(!m_db->skipKey(keyStr)){
                        // Create a new KeyValue object if it doesn't exist
                    
                    FV::KeyValue kv{};
                    kv.key = keyStr;
                    kv.value = valueStr;
                    kv.nodeIDs.push_back(id);
                    keyValues.push_back(kv);
                    }
                } 
            }
            
            if (tagIndex > i)
            {
                oldKeyIndex = i;
                break;
            }
        }
            */
        FV::Node node(id, lat, lon);
        nodes.push_back(node);
        // m_db->saveNode(id, lat, lon);

        //   std::cout << "Dense Node " << i << ": id=" << id
        //             << ", lat=" << lat
        //             << ", lon=" << lon
        //             << std::endl;
    }
    m_db->saveNodes(nodes);
    m_db->saveKeyValues(keyValues);
}

void FV::OSMWorker::processWay(const OSMPBF::PrimitiveGroup &primitiveGroup, OSMPBF::StringTable &stringTable)
{

    std::vector<FV::KeyValue> keyValues;
    std::vector<FV::NodeRef> refs;
    std::vector<int64_t> ways;

    for (size_t i = 0; i < primitiveGroup.ways_size(); i++)
    {
        const OSMPBF::Way &way = primitiveGroup.ways(i);
        ways.push_back(way.id());
        for (size_t j = 0; j < way.keys_size(); j++)
        {
            int32_t key = way.keys(j);
            std::string keyStr = stringTable.s(key);
            if (keyStr == "")
            {
                continue;
            }
            int32_t value = way.vals(j);
            std::string valueStr = stringTable.s(value);

              // Create a new KeyValue object if it doesn't exist
                        FV::KeyValue kv{};
                        kv.key = keyStr;
                        kv.value = valueStr;
                        kv.nodeID = 0;
                        kv.wayID = way.id();
                        kv.relationID = 0;
                        keyValues.push_back(kv);
            
        }

        uint64_t nodeID = 0;
        for (size_t j = 0; j < way.refs_size(); j++)
        {
            nodeID += way.refs(j);
            FV::NodeRef ref(way.id(), nodeID, j);
            refs.push_back(ref);
        }

        if (way.lat_size() > 0)
        {
            int bp = 0;
            bp++;
        }
    }

    if (ways.size() > 0)
    {
        m_db->saveWays(ways);
        m_db->saveKeyValues(keyValues);
        m_db->saveWayRefs(refs);
    }
}

void FV::OSMWorker::processRelations(const OSMPBF::PrimitiveGroup & primitiveGroup, OSMPBF::StringTable & stringTable)
{
    std::vector<int64_t> relationIDS;
    std::vector<FV::KeyValue> keyValues;
    std::vector<FV::RelationRole> relationRoles;

    for(size_t i = 0; i < primitiveGroup.relations_size(); i++){
        const OSMPBF::Relation &relation = primitiveGroup.relations(i);
        relationIDS.push_back(relation.id());

         for (size_t j = 0; j < relation.keys_size(); j++)
        {
            int32_t key = relation.keys(j);
            std::string keyStr = stringTable.s(key);
            if (keyStr == "")
            {
                continue;
            }
            int32_t value = relation.vals(j);
            std::string valueStr = stringTable.s(value);

             // Create a new KeyValue object if it doesn't exist
                        FV::KeyValue kv{};
                        kv.key = keyStr;
                        kv.value = valueStr;
                        kv.nodeID = 0;
                        kv.wayID = 0;
                        kv.relationID = relation.id();
                        keyValues.push_back(kv);
        }

        int64_t ref = 0;
        for(size_t j = 0; j < relation.roles_sid_size(); j++){
            int32_t role = relation.roles_sid(j);
            std::string roleStr = stringTable.s(role);
            if(roleStr == ""){
                continue;
            }
            ref += relation.memids(j);
            int64_t type = relation.types(j);
            FV::RelationRole relationRole(relation.id(), roleStr, ref, type);
            relationRoles.push_back(relationRole); 
        }
    }
    if(relationIDS.size() > 0){
        m_db->saveRelations(relationIDS);
        m_db->saveKeyValues(keyValues);
        m_db->saveRelationRoles(relationRoles);
    }
}
