//
// Created by Anaid Gakhokidze on 2017-05-08.
//

#ifndef TASKSCHEDULER_TASK_H
#define TASKSCHEDULER_TASK_H

#include <string>
#include <unordered_map>

class Task {
private:
    /*
     * Assumptions: - the output of the tasks is a map of metrics(doubles) with corresponding names.
     * */
    std::unordered_map<std::string, double> (*m_funcToExec)(void);

    // How often the task should be run, in seconds.
    long m_interval;

    // Name of the task, necessary to store metrics in the database under the name of the task
    std::string m_name;

public:
    Task(std::string name, std::unordered_map<std::string, double> (* funcToExec)(void), long interval)
            : m_funcToExec(funcToExec)
            , m_interval(interval)
            , m_name(name)
    {
    }

    void setInterval(long interval) { m_interval = interval; }

    long getInterval() { return m_interval; }

    std::string getName() { return m_name; }

    std::unordered_map<std::string, double> operator()()
    {
        return m_funcToExec();
    }

};



#endif //TASKSCHEDULER_TASK_H
