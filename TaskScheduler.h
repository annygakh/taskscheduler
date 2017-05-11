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
#include <unordered_map>

/*
 * Main class responsible for task scheduling.
 * It is through this class that users schedule tasks.
 * */
class TaskScheduler {
private:

    std::list<Task*> m_tasks;
    Database m_db;
    boost::lockfree::queue<Metric * > m_metricsQ;
    boost::atomic<bool> m_exit;
    Log m_log;
    bool m_updatedAggregateMetrics;
    std::unordered_map<std::string, int> m_tasksNumMetrics;

    void insertMetric(Metric & metric, std::unordered_map<std::string, int> & tableColsInited);

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

    void updateAggregateMetrics();

public:
    TaskScheduler();

    /*
     * Needs to be invoked after the constructor to initialize necessary objects.
     * */
    bool initialize();

    /*
     * Needs to be invoked before the destructor to deinitilize necessary objects.
     * */
    void deinitialize();

    /*
     * By default tasks will be run in the same order they were added.
     * */
    void addTask(Task * task);

    /*
     * Change the relative order in which it will execute compared to other tasks.
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

    ~TaskScheduler();

};


#endif //TASKSCHEDULER_TASKSCHEDULER_H
