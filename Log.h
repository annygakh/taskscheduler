//
// Created by Anaid Gakhokidze on 2017-05-10.
//

#ifndef TASKSCHEDULER_LOG_H
#define TASKSCHEDULER_LOG_H

#include <string>
#include <iostream>
#include <fstream>

class Log {
    std::string m_filename;
    FILE * m_logFile;

public:

    Log(std::string filename = "Log_file.txt")
            : m_filename(filename)
    {
        m_logFile = fopen(m_filename.c_str(), "w");
        if (m_logFile == NULL)
        {
            std::cerr << "Error opening the log file\n";
        }
    }

    void logMessage(const char* format, ...)
    {
        va_list argptr;
        va_start(argptr, format);
        vfprintf(m_logFile, format, argptr);
        va_end(argptr);
        fflush(m_logFile);
    }

    ~Log()
    {
        fclose(m_logFile);
    }
};



#endif //TASKSCHEDULER_LOG_H
