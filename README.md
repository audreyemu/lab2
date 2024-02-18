# You Spin Me Round Robin

Audrey Emis - 205997572

## Building
Run the following command to build the executable
```shell
make
```

## Running
Run the following command to run the executable, where processes.txt is a file where the first line is the number of processes and each line after contains the process ID, arrival time, and burst time separated by commas and # is a number representing the time slice length
```shell
./rr processes.txt #
```

The results from running ./rr processes.txt 3 with the given processes.txt file is printed as follows
```shell
Average waiting time: 7.00
Average response time: 2.75
```

## Cleaning up
Run the following command to clean up
```shell
make clean
```
