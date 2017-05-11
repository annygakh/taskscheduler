//
// Created by Anaid Gakhokidze on 2017-05-09.
//

#include "Database.h"
#include <sstream>


Database::Database()
    : m_dbName("introdb3.db")
    , m_recordsLog("Database_Records_log.txt")
    , m_stmLog("Database_Stm_log.txt")
{
}

bool Database::initialize()
{
    int rc;
    rc = sqlite3_open(m_dbName.c_str(), &m_database);
    return rc == SQLITE_OK;
}

bool Database::createTaskTable(std::string &tableName)
{
    std::string stm = "create table " + tableName + " ( " // TODO clean up the tableName - sql injection
            "id integer primary key AUTOINCREMENT, "
            "col1 real"
            ")";
    std::string errMsg = "cannot create table " + tableName;
    return executeStm(stm, errMsg);
}

bool Database::createAggregateMetricTable()
{
    std::string tableName = "aggregateMetrics";
    std::string stm = "create table " + tableName + " ( "
                    "taskName text, "
                    "metricType text, " /* min, max, average */
                    "colNum int, "
                    "value real,"
                    "primary key (taskName, metricType, colNum))";
    std::string errMsg = "cannot create table " + tableName;

    return executeStm(stm, errMsg);
}

bool Database::addColumn(std::string & tableName, std::string & colName)
{
    std::string stm = "ALTER TABLE " + tableName + " ADD " + colName + " real";
    std::string errMsg = "cannot add an extra column to table " + tableName;
    return  executeStm(stm, errMsg);
}

bool Database::updateAggregateMetric(std::string metricType, std::string & tableName, int colNum)
{
    std::stringstream stm;
    stm << "INSERT INTO aggregateMetrics SELECT ";
    stm << "\"" << tableName << "\",";
    stm << "\"" << metricType.c_str() << "\",";
    stm << colNum << ",";
    if (metricType == "avg")
    {
        stm << "round(";
    }
    stm << metricType.c_str();;

    stm << "(col" << colNum << ")";
    if (metricType == "avg")
    {
        stm << ", 2)";
    }
    stm << " from " << tableName;

    m_stmLog.logMessage("%s\n", stm.str().c_str());
    std::string errMsg = "cannot insert " + metricType + " value into aggregate metrics";
    std::string stmStr = stm.str();
    return  executeStm(stmStr, errMsg);
}

//bool Database::updateAllAggregateMetrics(std::map<std::string, int> & tasksCols)
//{
//    for (auto i : tasksCols)
//    {
//        std::string taskName = i.first;
//        int numCols = i.second;
//        for (int j = 0; j < numCols; j++)
//        {
//            updateAggregateMetric("avg", taskName, j + 1);
//            updateAggregateMetric("max", taskName, j + 1);
//            updateAggregateMetric("min", taskName, j + 1);
//        }
//    }
//    return true;
//}

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

std::string Database::constructInsertQuery(std::string & tableName, std::list<double> & values)
{
    std::stringstream ss;

    ss << "insert into " << tableName << " (";

    std::string separator = "";
    int j = 1;
    for (auto i = values.begin(); i!= values.end(); i++, j++)
    {

        ss << separator << "col" << j;
        separator = ",";

    }
    ss << ") values (";

    separator = "";
    for (auto i = values.begin(); i != values.end(); i++)
    {
        ss << separator << (*i);
        separator = ",";
    }
    ss << ")";
    return ss.str();
}

bool Database::insertRecord(std::string & tableName, std::list<double> & values)
{
    bool succ = true;
    sqlite3_stmt * sql;
    std::string statement = constructInsertQuery(tableName, values);

    int rc = sqlite3_prepare(m_database, statement.c_str(), statement.length(), &sql, 0);

    if (rc == SQLITE_OK)
    {
        int j = 0;
        for (auto i = values.begin(); i != values.end(); i++, j++)
        {
            sqlite3_bind_double(sql, j, (*i));
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
