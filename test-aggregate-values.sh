#!/bin/sh
# This script runs the taskscheduler with the database name you provide as the argument.
#  When the taskscheduler stops executing after a pre-programmed (by the script) number of seconds,
#  the script runs queries on that database
#  to ensure that aggregate values are all up to date.
#  You can run the script several times on the same database, to ensure that aggregate values get updated,
#  even after new metrics have been inserted.

#  - Set your script to be executable
#  `chmod +x test-aggregate-values.sh`
#  - Build your taskscheduler executable
#  - Run this script in the same folder where your executable is

# check num of args
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 nameOfDatabase" >&2
  exit 1
fi
DB_NAME=$1

echo "------ Running expect script. Input your pwd if prompted (for sudo mode) ------"
VAR=$(sudo expect -c'
spawn ./taskscheduler '$DB_NAME'
expect "Enter 'exit' to quit task scheduler: "
sleep 10
send "exit\r"
expect eof
catch wait result
')
echo "$VAR"

echo "------ Expect script completed. ------"

test_aggregate_value()
{
    task_name=$1
    metric_type=$2
    metric_name=$3
    output_aggregate="$(sqlite3 db.db 'select * from aggregateMetrics
        where taskName = "'"${task_name}"'"
        and metricType = "'"${metric_type}"'"
        and colName = "'"${metric_name}"'"' \
        | awk -F\| '{printf "%s\n", $4}')"
    if [ "$metric_type" = "avg" ]; then
        output_db="$(sqlite3 db.db 'select round(avg('"${metric_name}"'),2) from '"${task_name}"'')"
    else
        output_db="$(sqlite3 db.db 'select '"${metric_type}"'('"${metric_name}"') from '"${task_name}"'')"
    fi


    if [ "$output_aggregate" != "$output_db" ]; then
        echo "${metric_type} value for metric ${metric_name} for ${task_name} is NOT up to date"
    else
        echo "${metric_type} value for metric ${metric_name} for ${task_name} is up to date"
    fi
}

test_aggregate_value "connectToTcpServer" "avg" "timeElapsed"
test_aggregate_value "connectToTcpServer" "max" "timeElapsed"
test_aggregate_value "connectToTcpServer" "min" "timeElapsed"

test_aggregate_value "icmpPing" "avg" "timeElapsed"
test_aggregate_value "icmpPing" "max" "timeElapsed"
test_aggregate_value "icmpPing" "min" "timeElapsed"



