//
// Created by Anaid Gakhokidze on 2017-05-09.
//

#ifndef TASKSCHEDULER_METRIC_H
#define TASKSCHEDULER_METRIC_H

#include <list>
#include <string>

// This class is used to store information that needs to be inserted into the database.
class Metric {
private:
    std::string  m_taskName;
    std::unordered_map<std::string, double>  m_rawMetrics;
public:
    Metric() {}

    Metric(std::string taskName, std::unordered_map<std::string, double>rawMetrics)
            : m_taskName(taskName)
            , m_rawMetrics(rawMetrics)
    {
    }

    Metric(Metric *obj)
    {
        m_taskName = obj->getTaskName();
        m_rawMetrics = obj->getRawMetrics();
    }

    void setTaskName(std::string taskName) { m_taskName = taskName; }
    std::string getTaskName()const { return m_taskName; }

    void setRawMetrics(std::unordered_map<std::string, double> rawMetrics) { m_rawMetrics = rawMetrics; }
    std::unordered_map<std::string, double> getRawMetrics()const { return m_rawMetrics; }
};


#endif //TASKSCHEDULER_METRIC_H
