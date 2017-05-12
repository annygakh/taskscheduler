//
// Created by Anaid Gakhokidze on 2017-05-09.
//

#include "Database.h"
#include <sstream>


Database::Database(std::string databaseName)
    : m_dbName(databaseName)
    , m_recordsLog("Database_Records_log.txt")
    , m_stmLog("Database_Actions_log.txt")
{
}

bool Database::initialize()
{
    int rc;
    rc = sqlite3_open(m_dbName.c_str(), &m_database);
    return rc == SQLITE_OK;
}

bool Database::createTable(std::string &tableName)
{
    std::string stm = "create table if not exists " + tableName + " ( "
            "id integer primary key AUTOINCREMENT"
            ")";
    std::string errMsg = "cannot create table " + tableName;
    return executeStm(stm, errMsg);
}

bool Database::createAggregateMetricTable()
{
    std::string tableName = "aggregateMetrics";
    std::string stm = "create table if not exists " + tableName + " ( "
                    "taskName text, "
                    "metricType text, " /* min, max, average */
                    "colName text, "
                    "value real,"
                    "primary key (taskName, metricType, colName))";
    std::string errMsg = "cannot create table " + tableName;
    return executeStm(stm, errMsg);
}

bool Database::columnExists(std::string &tableName, std::string &colName)
{
    bool exists = false;
    std::string s = "pragma table_info('" + tableName + "')";
    sqlite3_stmt * stm;
    if ((sqlite3_prepare(m_database, s.c_str(), -1, &stm, 0) == SQLITE_OK))
    {
        int cols = sqlite3_column_count(stm);
        int result = 0;

        while(true)
        {
            result = sqlite3_step(stm);
            if (result == SQLITE_ROW)
            {
                std::stringstream currColName;
                currColName << sqlite3_column_text(stm, 1);
                if (currColName.str() == colName)
                {
                    m_stmLog.logMessage("Column %s exists. Will not alter table %s.\n", colName.c_str(), tableName.c_str());
                    exists = true;
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    sqlite3_finalize(stm);
    return exists;
}

bool Database::addColumn(std::string & tableName, std::string colName)
{
    if (columnExists(tableName, colName)) return true;

    std::string stm = "ALTER TABLE " + tableName + " ADD " + colName + " real";
    std::string errMsg = "cannot add an extra column to table " + tableName;
    return  executeStm(stm, errMsg);
}

bool Database::updateAggregateMetric(std::string metricType, std::string & tableName, std::string colName)
{
    std::stringstream stm;
    stm << "replace into aggregateMetrics SELECT ";
    stm << "\"" << tableName << "\",";
    stm << "\"" << metricType.c_str() << "\",";
    stm << "\'" << colName << "\',";
    if (metricType == "avg")
    {
        stm << "round(";
    }
    stm  << metricType.c_str();;

    stm << "(" << colName << ")";
    if (metricType == "avg")
    {
        stm << ", 2)";
    }
    stm << " from " << tableName;

    std::string errMsg = "cannot insert " + metricType + " value into aggregate metrics";
    std::string stmStr = stm.str();
    return  executeStm(stmStr, errMsg);
}

bool Database::executeStm(std::string & stm, std::string & error)
{
    bool succ = true;
    int responseCode;
    char *errMsg = 0;

    responseCode = sqlite3_exec(m_database, stm.c_str(), 0, 0, &errMsg);
    if (responseCode != SQLITE_OK)
    {
        m_stmLog.logMessage("SQL error: %s, %s\n", error.c_str(), errMsg);
        succ = false;
    }
    sqlite3_free(errMsg);
    return succ;
}

std::string Database::constructInsertQuery(std::string & tableName, std::unordered_map<std::string, double> & values)
{
    std::stringstream ss;
    ss << "insert into " << tableName << " (";

    std::string separator = "";
    int j = 1;

    for (auto i : values)
    {
        ss << separator << i.first;
        separator = ",";
        j++;
    }

    ss << ") values (";

    separator = "";

    for (auto i : values)
    {
        ss << separator << i.second;
        separator = ",";
    }

    ss << ")";
    return ss.str();
}

bool Database::insertRecord(std::string & tableName, std::unordered_map<std::string, double> & values)
{
    bool succ = true;
    sqlite3_stmt * sql;
    std::string statement = constructInsertQuery(tableName, values);

    int rc = sqlite3_prepare(m_database, statement.c_str(), statement.length(), &sql, 0);

    if (rc == SQLITE_OK)
    {
        int j = 0;
        for (auto i : values)
        {
            sqlite3_bind_double(sql, j, i.second);
            j++;
        }

        sqlite3_step(sql);
        sqlite3_finalize(sql);
    }
    else
    {
        m_recordsLog.logMessage(statement.c_str());
        succ = false;
    }
    return succ;
}

void Database::deinitialize()
{
    sqlite3_close(m_database);
}
