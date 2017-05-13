//
// Created by Anaid Gakhokidze on 2017-05-08.
//


#ifndef TASKSCHEDULER_TASKSCHEDULER_H
#define TASKSCHEDULER_TASKSCHEDULER_H

#include <list>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include <unordered_map>

#include "Task.h"
#include "Metric.h"
#include "Database.cpp"
#include "Log.h"

/*
 * Main class responsible for task scheduling.
 * It is through this class that users schedule tasks.
 * */
class TaskScheduler {
private:

    //
    // ----------- Variables
    //

    // Represents the mode of running task scheduler.
    // 0 - Interactively. Takes user input, and uses it to know when to terminate
    // 1 - Non interactive. stop() needs to be called to terminate the process
    int m_mode;

    // Contains tasks that need to be scheduled.
    std::list<Task*> m_tasks;

    // Database object.
    Database m_db;

    // Any metrics that needed to be added to the database, will be added to this Q.
    // The database thread continuously inserts these metrics into db.
    boost::lockfree::queue<Metric * > m_metricsQ;

    // Indicates whether task scheduler needs to terminate its execution.
    boost::atomic<bool> m_exit;

    // Log file
    Log m_log;

    // Indicates whether aggregate metrics have been updated in the database.
    bool m_updatedAggregateMetrics;

    // Stores the names of tasks and the corresponding metrics they output.
    // Needed for altering the tables for each task, or for updating aggregate metrics.
    std::unordered_map<std::string, std::unordered_map<std::string, double>> m_tasksMetrics;

    //
    // ----------- Methods
    //

    /*
     * Inserts a metric record into the database.
     * TODO remove tableColsInited
     * */
    void insertMetric(Metric & metric, std::unordered_map<std::string, int> & tableColsInited);

    /*
     * Routine designated for the thread responsible for reading the user's command.
     * */
    void userInputThreadTask();

    /*
     * Routine designated for the thread responsible for inserting metric data into the database.
     * It grabs metric data from the metric queue and inserts the data into the database.
     * */
    void databaseThreadTask();

    /*
     * Contains routines that each task's thread will have to execute.
     * Run and time the task, and save the metrics.
     * */
    void threadTask(Task * task);

    /*
     * Updates aggregate metrics, such as min, max, avg, for each metric of each task in the database.
     * */
    void updateAggregateMetrics();

public:
    TaskScheduler(std::string databaseName, int mode);

    /*
     * Needs to be invoked after adding tasks.
     * */
    bool initialize();

    /*
     * Needs to be invoked before the destructor.
     * */
    void deinitialize();

    /*
     * By default tasks will be run in the same order they were added.
     * */
    void addTask(Task * task);

    /*
     * Change the scheduling order of the task.
     * @param order - starts with 0 - means the task will be executed first
     * */
    void changeTaskOrder(Task * task, int order);

    /*
     * Cancel the task that has previously been added to task scheduler.
     * */
    void cancelTask(Task * task);

    /*
     * Start the task scheduler.
     * */
    void start();

    /*
     * Stops the task scheduler if not running it interactively.
     * */
    void stop();

};


#endif //TASKSCHEDULER_TASKSCHEDULER_H
