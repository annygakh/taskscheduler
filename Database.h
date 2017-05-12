//
// Created by Anaid Gakhokidze on 2017-05-09.
//

#ifndef TASKSCHEDULER_DATABASE_H
#define TASKSCHEDULER_DATABASE_H
#include <string>
#include <sqlite3.h>
#include "Log.h"
#include <list>

class Database {
private:
    /*
     * sqlite3 database connection.
     * */
    sqlite3 * m_database;

    /*
     * Name of the database.
     * */
    std::string m_dbName;

    /*
     * Log file that stores raw sql queries that failed to be executed by the database
     * when trying to insert a record.
     * */
    Log m_recordsLog;

    /*
     * Log file that contains actions that database failed to execute,
     * such as failing to create a table or alter a table.
     * */
    Log m_stmLog;

    /*
     * Construct a query in the following format
     * "insert into tableName (col1.... colN) values (val1.... valN)"
     * */
    std::string constructInsertQuery(std::string & tableName, std::list<double> & values);

    /*
     * Execute a sqlite statement.
     * @param error - what the error message in the log should contain.
     * */
    bool executeStm(std::string & stm, std::string & error);

public:
    Database(std::string databaseName);

    /*
     * Returns true if a database connection has been established successfully, false otherwise.
     * The user is responsible for calling deinitialize() even if initialize() returned false.
     * Needs to be invoked after the constructor.
     * */
    bool initialize();

    /*
     * Needs to be invoked before the destructor.
     * */
    void deinitialize();

    /*
     * Insert a record with 'values' into 'tableName'.
     * */
    bool insertRecord(std::string & tableName, std::list<double> & values);

    /*
     * Create a table 'tableName' with an initial extra column for one metric.
     * */
    bool createTable(std::string &tableName);

    /*
     * Create a table to store aggregate metrics.
     * */
    bool createAggregateMetricTable();

    /*
     * Update aggregate metrics.
     * */
    bool updateAggregateMetric(std::string metricType, std::string & tableName, int colNum);

    /*
     * Alter a table to add an extra column.
     * */
    bool addColumn(std::string & tableName, std::string & colName);
};


#endif //TASKSCHEDULER_DATABASE_H
