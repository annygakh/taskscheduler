//
// Created by Anaid Gakhokidze on 2017-05-08.
//

#ifndef TASKSCHEDULER_TASK_H
#define TASKSCHEDULER_TASK_H

#include <string>

class Task {
private:
    /*
     * Assumptions: - the output of the tasks is simply a list of doubles,
     *              representing different metrics. This can easily be changed into a map,
     *              so that each metric has a corresponding name.
     * */
    std::list<double> (*m_funcToExec)(void);
    long m_interval;
    std::string m_name;

public:
    Task(std::string name, std::list<double> (* funcToExec)(void), long interval)
            : m_funcToExec(funcToExec)
            , m_interval(interval)
            , m_name(name)
    {
    }

    void setInterval(long interval) { m_interval = interval; }

    long getInterval() { return m_interval; }

    std::string getName() { return m_name; }

    std::list<double> operator()()
    {
        return m_funcToExec();
    }

};



#endif //TASKSCHEDULER_TASK_H
