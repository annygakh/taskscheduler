#include <iostream>

#include "TaskScheduler.cpp"
#include <list>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <chrono>

#include "Ping.cpp"

double foo1 = 0, bar1 = 0, bar2 = 0.6;
std::list<double> foo(void)
{
    std::list<double> metrics;
    metrics.push_back(foo1++);
    return metrics;
}
std::list<double> bar(void)
{
    std::list<double> metrics;
    metrics.push_back(bar1++);
    metrics.push_back(bar2++);
    return metrics;
}

// Forward declarations
std::list<double> connectToTcpServer(void);
void destroy();
void sigIntHandler (int signum);


TaskScheduler * ts;
Task * t1, * t2, * t3, *t4;

int main(int argc, char ** argv) {
    if (argc < 2)
    {
        std::cerr << " Usage: taskscheduler databaseName.db\n";
        exit(1);
    }

    ts = new TaskScheduler(argv[1]);

    signal(SIGABRT, sigIntHandler);
    signal(SIGINT, sigIntHandler);
    signal(SIGTERM, sigIntHandler);

    t1 = new Task("foo", foo, 2);
    t2 = new Task("bar", bar, 4);
    t3 = new Task("connectToTcpServer", connectToTcpServer, 5);
    t4 = new Task("icmpPing", &Ping::ping, 7);
    ts->addTask(t1);
    ts->addTask(t2);
    ts->addTask(t3);
    ts->addTask(t4);

    ts->changeTaskOrder(t4, 0); // uncomment this to change the order of tasks
    ts->cancelTask(t1); // uncomment this to cancel a task

    if (!ts->initialize())
    {
        std::cerr << "Failed to initialize task scheduler. Check the logs for details.\n";
        ts->deinitialize();
    }
    else
    {
        ts->start();
    }
    destroy();
    return 0;
}

// Handles deinitialization of task scheduler, and deletion of created objects
void destroy()
{
    ts->deinitialize();
    delete ts;
    delete t1;
    delete t2;
    delete t3;
    std::cout << "Task scheduler terminated successfully.\n";
}


void sigIntHandler (int signum)
{
    destroy();
    std::cout << "Task scheduler terminated.\n";
    exit(0);
}

/*
 * Outputs the number of microseconds it took to connect to google.com at port 80
 * */
std::list<double> connectToTcpServer(void)
{
    bool succ = true;
    std::chrono::steady_clock::time_point start, end;
    std::string host = "google.com", portNo = "80";

    int rc = -1;
    struct addrinfo hints, * servinfo, *pt /* protocol */;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    while (true)
    {
        start = std::chrono::steady_clock::now();
        if ((rc = getaddrinfo(host.c_str(), portNo.c_str(), &hints, &servinfo)) != 0)
        {
            succ = false;
            break; // abort the loop
        }

        int sockfd = -1;

        for (pt = servinfo; pt != NULL; pt = pt->ai_next)
        {
            if ((sockfd = socket(pt->ai_family, pt->ai_socktype, pt->ai_protocol)) < 0)
            {
                continue;
            }

            if (connect(sockfd, pt->ai_addr, pt->ai_addrlen) < 0)
            {
                close(sockfd);
                sockfd = -1;
                continue;
            }

            break; // got one
        }

        if (pt == NULL)
        {
            succ = false;
            break; // abort the loop
        }

        break; // abort the loop
    }
    double timeElapsed;
    if (succ)
    {
        end = std::chrono::steady_clock::now();
        timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }
    else
    {
        timeElapsed = 0; // this way if the timeElapsed is 0, we know that tcp connection failed
    }
    std::list<double> metrics;
    metrics.push_back(timeElapsed);

    freeaddrinfo(servinfo);
    return metrics;
}