//
// Created by Anaid Gakhokidze on 2017-05-08.
//

#include <thread>
#include <vector>
#include <sstream>

#include "TaskScheduler.h"
#include "Log.h"


TaskScheduler::TaskScheduler(std::string databaseName, int mode)
    : m_mode(mode)
    , m_tasks()
    , m_db(databaseName)
    , m_metricsQ(0) // Initial size is 0, but the Q is variable in size
    , m_exit(false)
    , m_log()
    , m_updatedAggregateMetrics(false)
    , m_tasksMetrics()
{
}

bool TaskScheduler::initialize()
{
    if (!m_db.initialize()) return false;
    for (auto i = m_tasks.begin(); i != m_tasks.end(); i++)
    {
        std::string taskName = (*i)->getName();
        if (!m_db.createTable(taskName))
        {
            return false;
        }
    }
    if (!m_db.createAggregateMetricTable())
    {
        return false;
    }
    return true;
}

void TaskScheduler::addTask(Task *task)
{
    m_tasks.push_back(task);
}

void TaskScheduler::cancelTask(Task *task)
{
    m_tasks.remove(task);
}

void TaskScheduler::changeTaskOrder(Task *task, int order)
{
    m_tasks.remove(task);
    std::list<Task*>::iterator itr = m_tasks.begin();
    std::advance(itr, order);
    m_tasks.insert(itr, task);
}

void TaskScheduler::threadTask(Task * task)
{
    m_log.logMessage("Beginning the thread for task %s\n", task->getName().c_str());
    while (!m_exit)
    {
        std::unordered_map<std::string, double> metrics = task->operator()();
        m_log.logMessage("%s()\n", task->getName().c_str());

        Metric * metric = new Metric(task->getName(), metrics);
        m_metricsQ.push(metric);

        std::this_thread::sleep_for(std::chrono::seconds(task->getInterval()));
    }
}

void TaskScheduler::insertMetric(Metric & metric)
{
    std::string taskName = metric.getTaskName();
    std::unordered_map<std::string, double> rawMetrics = metric.getRawMetrics();
    bool needToAlterTable = m_tasksMetrics.find(taskName) == m_tasksMetrics.end();
    if (needToAlterTable)
    {
        for (auto i : rawMetrics)
        {
            m_db.addColumn(taskName, i.first);
        }
        m_tasksMetrics.insert({taskName, rawMetrics});
    }
    m_db.insertRecord(taskName, rawMetrics);
}

void TaskScheduler::databaseThreadTask()
{
    Metric metric;
    while (!m_exit)
    {
        while (m_metricsQ.pop(metric))
        {
            insertMetric(metric);
        }
    }

    while (m_metricsQ.pop(metric))
    {
        insertMetric(metric);
    }
    m_log.logMessage("Finished inserting all of the outstanding metrics into the database\n");

    updateAggregateMetrics();
}

void TaskScheduler::userInputThreadTask()
{
    std::cout << "Task scheduler is running now.\n";

    std::string userCommand;
    do {
        std::cout << "Enter 'exit' or Ctrl-C and press enter to quit task scheduler: ";
        std::cin >> userCommand;
    } while (userCommand != "exit" && !m_exit);

    std::cout << "Exiting task scheduler\n";
    m_exit = true;
}


void TaskScheduler::start()
{
    m_log.logMessage("Starting task scheduler\n");
    std::thread dbThread(&TaskScheduler::databaseThreadTask, this);
    std::thread userInputThread;
    if (m_mode == 0)
    {
        userInputThread = std::thread(&TaskScheduler::userInputThreadTask, this);
    }

    std::vector<std::thread> threads;
    for (auto i = m_tasks.begin(); i != m_tasks.end(); i++)
    {
        threads.push_back(std::thread(&TaskScheduler::threadTask,  this, *i));
    }
    for (auto i = threads.begin(); i != threads.end(); i++)
    {
        (*i).join();
    }
    dbThread.join();
    if (m_mode == 0)
    {
        userInputThread.join();
    }
}



void TaskScheduler::updateAggregateMetrics()
{
    if (m_updatedAggregateMetrics) return;

    for (auto i : m_tasksMetrics)
    {
        std::string taskName = i.first;
        for (auto j : i.second)
        {
            m_db.updateAggregateMetric("avg", taskName, j.first);
            m_db.updateAggregateMetric("max", taskName, j.first);
            m_db.updateAggregateMetric("min", taskName, j.first);

        }
    }
    m_updatedAggregateMetrics = true;
    m_log.logMessage("Finished updating all of the aggregate values for metrics.\n");
}

void TaskScheduler::deinitialize()
{
    if (!m_updatedAggregateMetrics)
    {
        updateAggregateMetrics();
    }
    m_db.deinitialize();
    m_log.logMessage("Task scheduler de-initialized successfully.\n");
}

void TaskScheduler::stop()
{
    m_exit = true;
    m_log.logMessage("Stop command has been initiated\n");
}
