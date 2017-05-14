# Task Scheduler
Simple periodic task-scheduler written in C++. Tasks are run on separate, configurable intervals.


## Usage

```
./taskscheduler databaseName.db
# or run with sudo, to execute icmp ping
sudo ./taskscheduler databaseName.db
``` 
At any point you can exit using `Ctrl-C` (the task scheduler will exit gracefully)
or by typing `exit` when prompted.

## Documentation

### Embedding Task Scheduler in your code
`main.cpp` provides an example of how to use TaskScheduler class in your program, 
- To embed task scheduler in your code, you have to put the following in your file
    ```
    #include "TaskScheduler.cpp"
    ```
    You also have to link all of the files task scheduler uses.
- Tasks are scheduled by creating an instance of class Task and adding it to Task Scheduler.
    You can cancel or reorder tasks, after adding them to the task scheduler.
- A task is a function with the following signature 
    ```
        std::unordered_map<std::string, double> yourTask(void);
    ```
- Output of a task is a map of one or more metrics (decimal values) with corresponding names
    e.g.
  - timeElapsed - 30.5
  - packetsSent - 23
  - packetsReceived - 21
- Task scheduler can be run interactively (i.e. needs user input to terminate) or non-interactively 
(where it can be terminated at any time by calling stop() from some callback)
        


### Database
- Task scheduler uses sqlite3 database to store all of the information.
- Each task will have a corresponding table, 
where each metric will have its own column.
- Aggregate values (min, max, avg) 
of each metric will be stored in table `aggregateMetrics`
  - ```
    taskName | valueType | metricName | value
    ```
  - valueType is one of max, min, avg
- If the task scheduler is run again using the same database, 
new metrics will be appended to existing tables, 
and aggregate values of each metric will be updated accordingly.

### Log files
- Any metrics that failed to be inserted into the database,
will be logged in Database_Records_log.txt
- Any actions that fail to be executed by the database, 
will be logged in Database_Actions_log.txt
- Any other messages will be logged in Log_file.txt

## Development
All the development was done on macOS Sierra 10.12.2
### Prerequisite libraries
You will need the following installed
- boost
- sqlite3

### Building
The CMakeLists.txt is provided for you.


### Handling issues
Check the log files to locate the source of error.
 
Known issues and caveats
- When the `taskscheduler` is not run with sudo, ICMP ping will not take place.
The elapsedTime for task icmpPing in this case will be 0.0.
- If there was an error with executing ICMP ping task or connecting to a TCP server, 
such as an error with creation of a socket, or with a connection of a socket, the timeElapsed will be 0.0
to reflect that the task did not complete successfully.

## Testing
There are currently no unit tests. Besides adhering to the 'testing-as-I-go' approach, 
I have tested the program by launching the `taskscheduler` and inspecting packets in WireShark.
I was looking at the logs
```
tail -f -n 50 Log_file.txt
```
and ensuring that whenever icmpPing() was printed in the log file, a new ICMP packet was shown in WireShark.

Similarly, I watched the TCP packets for connectToTcpServer().

### Testing aggregate values
There is a script `test-aggregate-values.sh` that runs the taskscheduler with the database name you provide as the argument.
When the taskscheduler stops executing after a pre-programmed (by the script) number of seconds, the script runs queries on that database 
to ensure that aggregate values are all up to date. 
You can run the script several times on the same database, to ensure that aggregate values get updated,
even after new metrics have been inserted.

- Set your script to be executable
`chmod +x test-aggregate-values.sh`
- Build your taskscheduler executable
- Run this script in the same folder where your executable is

## Authors
Anny Gakhokidze

## License
TBD

## Acknowledgements
