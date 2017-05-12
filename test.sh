#!/bin/sh

# FIRST build your taskscheduler executable and cd into the directory where it is

# check num of args
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 nameOfDatabase" >&2
  exit 1
fi
DB_NAME=$1

# remove existing logs and dbs
rm -f $1
rm -f Database_Actions_log.txt
rm -f Database_Records_log.txt
rm -f Log_file.txt

echo "------ Running expect script. Do not input anything. ------"
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



