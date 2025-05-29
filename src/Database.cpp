#include "Database.h"

FV::Database::Database(std::string dbPath)
    : m_dbPath{dbPath}
{
    initializeDB();
    createSkipKeys();
}

FV::Database::~Database()
{
    if (m_db)
    {
        sqlite3_close(m_db);
    }
}

void FV::Database::saveNodes(std::vector<FV::Node> &nodes)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    sqlite3_stmt *stmt = 0;
    char *errMsg;
    int rc = sqlite3_prepare_v2(m_db, "INSERT INTO NODE(ID, LAT, LON) VALUES(?,?,?)", -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Error in preparing SQL statement: %s \n", sqlite3_errmsg(m_db));
        return;
    }
    //  Optional, but will most likely increase performance.
    rc = sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, 0);
    for (int bindIndex = 0; bindIndex < nodes.size(); bindIndex++)
    {
        //  Binding integer values in this example.
        //  Bind functions for other data-types are available - see end of post.

        //  Bind-parameter indexing is 1-based.
        rc = sqlite3_bind_int(stmt, 1, nodes[bindIndex].id);  // Bind first parameter.
        rc = sqlite3_bind_double(stmt, 2, nodes[bindIndex].lat); // Bind second parameter.
        rc = sqlite3_bind_double(stmt, 3, nodes[bindIndex].lon); // Bind second parameter.

        //  Reading interger results in this example.
        //  Read functions for other data-types are available - see end of post.

        //  Step, Clear and Reset the statement after each bind.
        rc = sqlite3_step(stmt);
        rc = sqlite3_clear_bindings(stmt);
        rc = sqlite3_reset(stmt);
    }

    char *zErrMsg = 0;                                          //  Can perhaps display the error message if rc != SQLITE_OK.
    rc = sqlite3_exec(m_db, "END TRANSACTION", 0, 0, &zErrMsg); //  End the transaction.
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", zErrMsg);
        // free the error message
        sqlite3_free(zErrMsg);
    }

    rc = sqlite3_finalize(stmt); //  Finalize the prepared statement.
}

void FV::Database::saveKeyValues(std::vector<FV::KeyValue> &keyValues)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    { // INSERT
        sqlite3_stmt *stmt = 0;
        char *errMsg;
        int rc = sqlite3_prepare_v2(m_db, "INSERT INTO KeyValue(KEY, VAL, NodeID, WayID, RelationID) VALUES(?, ?, ?,?,?)", -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Error in preparing SQL statement: %s \n", sqlite3_errmsg(m_db));
            return;
        }
        //  Optional, but will most likely increase performance.
        rc = sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, 0);
        for (int bindIndex = 0; bindIndex < keyValues.size(); bindIndex++)
        {
            
            rc = sqlite3_bind_text(stmt, 1, keyValues[bindIndex].key.c_str(), -1, SQLITE_STATIC);   // Bind fourth parameter.
            rc = sqlite3_bind_text(stmt, 2, keyValues[bindIndex].value.c_str(), -1, SQLITE_STATIC); // Bind fifth parameter.
            rc = sqlite3_bind_int64(stmt, 3, keyValues[bindIndex].nodeID); // Bind sixth parameter.
            rc = sqlite3_bind_int64(stmt, 4, keyValues[bindIndex].wayID);  // Bind seventh parameter.
            rc = sqlite3_bind_int64(stmt, 5, keyValues[bindIndex].relationID); // Bind eighth parameter. 
         

            //  Step, Clear and Reset the statement after each bind.
            rc = sqlite3_step(stmt);
            rc = sqlite3_clear_bindings(stmt);
            rc = sqlite3_reset(stmt);
        }

        char *zErrMsg = 0;                                          //  Can perhaps display the error message if rc != SQLITE_OK.
        rc = sqlite3_exec(m_db, "END TRANSACTION", 0, 0, &zErrMsg); //  End the transaction.
        if (rc != SQLITE_OK)
        {
            printf("Error in executing SQL: %s \n", zErrMsg);
            // free the error message
            sqlite3_free(zErrMsg);
        }

        rc = sqlite3_finalize(stmt); //  Finalize the prepared statement.
    } // INSERT
 
}

void FV::Database::saveWays(std::vector<int64_t> &wayIDS)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    sqlite3_stmt *stmt = 0;
    char *errMsg;
    int rc = sqlite3_prepare_v2(m_db, "INSERT INTO Way(WayID) VALUES(?)", -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Error in preparing SQL statement: %s \n", sqlite3_errmsg(m_db));
        return;
    }
    //  Optional, but will most likely increase performance.
    rc = sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, 0);
    for (int bindIndex = 0; bindIndex < wayIDS.size(); bindIndex++)
    {
        rc = sqlite3_bind_int64(stmt, 1, wayIDS[bindIndex]); // Bind first parameter.

        //  Step, Clear and Reset the statement after each bind.
        rc = sqlite3_step(stmt);
        rc = sqlite3_clear_bindings(stmt);
        rc = sqlite3_reset(stmt);
    }

    char *zErrMsg = 0;                                          //  Can perhaps display the error message if rc != SQLITE_OK.
    rc = sqlite3_exec(m_db, "END TRANSACTION", 0, 0, &zErrMsg); //  End the transaction.
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", zErrMsg);
        // free the error message
        sqlite3_free(zErrMsg);
    }

    rc = sqlite3_finalize(stmt); //  Finalize the prepared statement.
}

void FV::Database::saveWayRefs(std::vector<FV::NodeRef> &refs)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    sqlite3_stmt *stmt = 0;
    char *errMsg;
    int rc = sqlite3_prepare_v2(m_db, "INSERT INTO WayRef(wayID, nodeID, refIndex) VALUES(?,?,?)", -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Error in preparing SQL statement: %s \n", sqlite3_errmsg(m_db));
        return;
    }
    //  Optional, but will most likely increase performance.
    rc = sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, 0);
    for (int bindIndex = 0; bindIndex < refs.size(); bindIndex++)
    {
        rc = sqlite3_bind_int64(stmt, 1, refs[bindIndex].wayID);    // Bind first parameter.
        rc = sqlite3_bind_int64(stmt, 2, refs[bindIndex].nodeID);   // Bind second parameter.
        rc = sqlite3_bind_int64(stmt, 3, refs[bindIndex].refIndex); // Bind third parameter.

        //  Step, Clear and Reset the statement after each bind.
        rc = sqlite3_step(stmt);
        rc = sqlite3_clear_bindings(stmt);
        rc = sqlite3_reset(stmt);
    }

    char *zErrMsg = 0;                                          //  Can perhaps display the error message if rc != SQLITE_OK.
    rc = sqlite3_exec(m_db, "END TRANSACTION", 0, 0, &zErrMsg); //  End the transaction.
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", zErrMsg);
        // free the error message
        sqlite3_free(zErrMsg);
    }

    rc = sqlite3_finalize(stmt); //  Finalize the prepared statement.
}

void FV::Database::saveRelations(std::vector<int64_t> &relationIDS)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    sqlite3_stmt *stmt = 0;
    char *errMsg;
    int rc = sqlite3_prepare_v2(m_db, "INSERT INTO Relation(RelationID) VALUES(?)", -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Error in preparing SQL statement: %s \n", sqlite3_errmsg(m_db));
        return;
    }
    //  Optional, but will most likely increase performance.
    rc = sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, 0);
    for (int bindIndex = 0; bindIndex < relationIDS.size(); bindIndex++)
    {
        rc = sqlite3_bind_int64(stmt, 1, relationIDS[bindIndex]); // Bind first parameter.

        //  Step, Clear and Reset the statement after each bind.
        rc = sqlite3_step(stmt);
        rc = sqlite3_clear_bindings(stmt);
        rc = sqlite3_reset(stmt);
    }

    char *zErrMsg = 0;                                          //  Can perhaps display the error message if rc != SQLITE_OK.
    rc = sqlite3_exec(m_db, "END TRANSACTION", 0, 0, &zErrMsg); //  End the transaction.
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", zErrMsg);
        // free the error message
        sqlite3_free(zErrMsg);
    }

    rc = sqlite3_finalize(stmt); //  Finalize the prepared statement.
}

void FV::Database::saveRelationRoles(std::vector<FV::RelationRole> &relationRoles)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    sqlite3_stmt *stmt = 0;
    char *errMsg;
    int rc = sqlite3_prepare_v2(m_db, "INSERT INTO RelationMember(RelationID, Role, RefIndex, MemberType) VALUES(?, ?, ?, ?)", -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Error in preparing SQL statement: %s \n", sqlite3_errmsg(m_db));
        return;
    }
    //  Optional, but will most likely increase performance.
    rc = sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, 0);
    for (int bindIndex = 0; bindIndex < relationRoles.size(); bindIndex++)
    {
        rc = sqlite3_bind_int64(stmt, 1, relationRoles[bindIndex].relationID);                     // Bind first parameter.
        rc = sqlite3_bind_text(stmt, 2, relationRoles[bindIndex].role.c_str(), -1, SQLITE_STATIC); // Bind second parameter.
        rc = sqlite3_bind_int64(stmt, 3, relationRoles[bindIndex].memberID);                       // Bind third parameter.
        rc = sqlite3_bind_int64(stmt, 4, relationRoles[bindIndex].memberType);                     // Bind fourth parameter.

        //  Step, Clear and Reset the statement after each bind.
        rc = sqlite3_step(stmt);
        rc = sqlite3_clear_bindings(stmt);
        rc = sqlite3_reset(stmt);
    }

    char *zErrMsg = 0;                                          //  Can perhaps display the error message if rc != SQLITE_OK.
    rc = sqlite3_exec(m_db, "END TRANSACTION", 0, 0, &zErrMsg); //  End the transaction.
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", zErrMsg);
        // free the error message
        sqlite3_free(zErrMsg);
    }

    rc = sqlite3_finalize(stmt); //  Finalize the prepared statement.
}

bool FV::Database::skipKey(std::string &key)
{
    for (size_t i = 0; i < m_skipKeys.size(); i++)
    {
        if (m_skipKeys[i] == key)
        {
            return true; // Skip this key
        }
    }
    return false;
}

uint64_t FV::Database::getKeyIndex(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    uint64_t id = 0;
    {
        sqlite3_stmt *stmt = 0;
        char *errMsg;
        int rc = sqlite3_prepare_v2(m_db, "INSERT OR IGNORE INTO Keys(KEY) VALUES(?)", -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Error in preparing SQL statement: %s \n", sqlite3_errmsg(m_db));
            return 0;
        }
        rc = sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC); // Bind first parameter.
        switch (sqlite3_step(stmt))
        {
        case SQLITE_ROW: // case sqlite3_step has another row ready
        {
          //  id = sqlite3_column_int64(stmt, 0); // Get the ID of the key
            sqlite3_finalize(stmt);  
                       // d
        }
         break;
        case SQLITE_DONE: // case sqlite3_step() has finished executing
        {
          //  id = sqlite3_last_insert_rowid(m_db); // Get the last inserted row ID
            sqlite3_finalize(stmt);               // d
          //  return id;
        }
        break;
        default:
        {
            printf("Error in executing SQL: %s \n", sqlite3_errmsg(m_db));
            sqlite3_finalize(stmt); // destroy
        }
        }
    }

     {
        sqlite3_stmt *stmt = 0;
        char *errMsg;
        int rc = sqlite3_prepare_v2(m_db, "SELECT ID FROM KEYS WHERE KEY = ?", -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Error in preparing SQL statement: %s \n", sqlite3_errmsg(m_db));
            return 0;
        }
        rc = sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC); // Bind first parameter.
        switch (sqlite3_step(stmt))
        {
        case SQLITE_ROW: // case sqlite3_step has another row ready
        {
            id = sqlite3_column_int64(stmt, 0); // Get the ID of the key
            sqlite3_finalize(stmt);  
            return id;           // d
        }
        case SQLITE_DONE: // case sqlite3_step() has finished executing
        {
            id = sqlite3_last_insert_rowid(m_db); // Get the last inserted row ID
            sqlite3_finalize(stmt);               // d
        }
        default:
        {
            printf("Error in executing SQL: %s \n", sqlite3_errmsg(m_db));
            sqlite3_finalize(stmt); // destroy
        }
        }
    }

    return id;
}

void FV::Database::initializeDB()
{
    m_openResult = sqlite3_open(m_dbPath.c_str(), &m_db);
    if (m_openResult != SQLITE_OK)
    {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(m_db)));
    }

    char *errMsg;
    std::string sql = "CREATE TABLE IF NOT EXISTS NODE("
                      "ID INT PRIMARY KEY NOT NULL,"
                      "LAT REAL NOT NULL,"
                      "LON REAL NOT NULL)";

    int rc = sqlite3_exec(m_db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", errMsg);
        // free the error message
        sqlite3_free(errMsg);
    }
    else
    {
        printf("table NODE made successfully");
    }

    sql = "CREATE TABLE IF NOT EXISTS Keys("
          "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
          "KEY TEXT UNIQUE NOT NULL)";

    rc = sqlite3_exec(m_db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", errMsg);
        // free the error message
        sqlite3_free(errMsg);
    }
    else
    {
        printf("table Keys made successfully");
    }

    sql = "CREATE TABLE IF NOT EXISTS KeyValue("
          "KEY TEXT NOT NULL,"
          "VAL TEXT NOT NULL, "
          "NodeID INT NOT NULL, "
          "WayID INT NOT NULL, "
          "RelationID INT NOT NULL)";

    rc = sqlite3_exec(m_db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", errMsg);
        // free the error message
        sqlite3_free(errMsg);
    }
    else
    {
        printf("table KeyValue made successfully");
    }

    sql = "CREATE TABLE IF NOT EXISTS Way("
          "WayID INT PRIMARY KEY NOT NULL)";

    rc = sqlite3_exec(m_db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", errMsg);
        // free the error message
        sqlite3_free(errMsg);
    }
    else
    {
        printf("table Way made successfully");
    }

    sql = "CREATE TABLE IF NOT EXISTS WayRef("
          "wayID INT KEY NOT NULL, "
          "nodeID INT NOT NULL, "
          "refIndex INT NOT NULL)";

    rc = sqlite3_exec(m_db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", errMsg);
        // free the error message
        sqlite3_free(errMsg);
    }
    else
    {
        printf("table WayRef made successfully");
    }

    sql = "CREATE TABLE IF NOT EXISTS Relation("
          "RelationID INT PRIMARY KEY NOT NULL)";

    rc = sqlite3_exec(m_db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", errMsg);
        // free the error message
        sqlite3_free(errMsg);
    }
    else
    {
        printf("table Way made successfully");
    }

    sql = "CREATE TABLE IF NOT EXISTS RelationMember("
          "relationID INT KEY NOT NULL, "
          "Role TEXT NOT NULL, "
          "RefIndex INT NOT NULL, "
          "MemberType INT NOT NULL)";

    rc = sqlite3_exec(m_db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error in executing SQL: %s \n", errMsg);
        // free the error message
        sqlite3_free(errMsg);
    }
    else
    {
        printf("table WayRef made successfully");
    }
}

void FV::Database::createSkipKeys()
{
    m_skipKeys.push_back("created_by");
    m_skipKeys.push_back("source");
    m_skipKeys.push_back("version");
    m_skipKeys.push_back("timestamp");
    m_skipKeys.push_back("changeset");
    m_skipKeys.push_back("user");
    m_skipKeys.push_back("website");
    m_skipKeys.push_back("phone");
    m_skipKeys.push_back("opening_hours");
    m_skipKeys.push_back("contact:email");
    m_skipKeys.push_back("contact:website");
    m_skipKeys.push_back("contact:phone");
    m_skipKeys.push_back("contact:mobile");
    m_skipKeys.push_back("contact:fax");
    m_skipKeys.push_back("contact:facebook");
    m_skipKeys.push_back("fax");
    m_skipKeys.push_back("phone");
    m_skipKeys.push_back("check_date:opening_hours");
    m_skipKeys.push_back("check_date");

    // Add more keys to skip as needed
}
