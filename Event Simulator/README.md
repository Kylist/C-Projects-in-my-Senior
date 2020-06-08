Abstraction:
1. The program is created with the purpose of simulating a simple computing system with 1 CPU and 2 Disks. The process of the program will include 4 steps. First, the program will take input from configuration file (config.txt) containing a seed for the random function and the configurations. Second, the program generates and handles events with priority queue for the event input and 3 FIFO queues for the CPU and 2 Disks. Third, the configurations of the program and the event queue log will be outputted to a log.txt file. Finally, the statistics of the components (average and the maximum size of each queue, utilization of each server) will be displayed to the screen.
2. The diagram of the program is described below.

Alternatives in design & Testing:
1. The PROB_QUIT configuration in my program is in percentage (so 0.2 would be 20 (%))
2. I implement a priority enqueue function to the queue structures of the CPU and disks. Therefore, I can use that queue structure for the event queue too. So, I don’t have to build another priority queue structure for the event queue. 
3. The program is tested in Linux with different information changed in the config.txt. There are 4 times of testing. Two of the first one, only the Seeds will be changed. In the third one, FIN time will be change. And for the last one, all the max data will be added by 5. The more detail will be described in RUNS.txt. 

Files included in the program:
1. main.c: Contains the main function, the simulation functions and queue functions. 
2. config.txt: containing the configurations for the program (SEED, INIT_TIME, FIN_TIME). 
3. Log.txt: containing the output of the program
* RUNS.txt: this file is not in the program. However, this describe the test runs and my comments after the test runs.

Program Explanation:
The main file “main.c” contains the simulation, handler functions and the queue structures, along with the file I/O inside the main function. It first takes input from the configurations file “config.txt” into an array of integer (that will be used by the functions later), then write the configurations into the log.txt file. The simulation function then creates the 4 queues (event, CPU, disk 1, disk 2). Then it loads the first event into the event queue. After that, it begins to dequeue the event queue until either the queue is empty (should not happen before reaching finish time) or the finish time FIN_TIME has been reached. For each type of event, a handler function is used. The log for the event generating and handling process is written into the log.txt file. During the process, the handler functions also keep track of the components statistics by modifying the statistic pointers each time it is called. After the final event Simulation finishes has been reached, it will print out the statistics to the screen. For the queue structures, they include 3 structures, which are event, node, and queue:
1. An event is a structure containing an integer for the job number, an array of character for the event description, and an integer for the time that the event occurs. 
2. A node is defined to contain an event and a pointer that links to another node. The implemented queue contains 2 nodes, head and tail, along with an integer variable to get the queues size. 
3. The files also contain the standard functions for the queue data structure (enqueue, dequeue, get size), the constructor functions (newEvent, newNode), and a function to print out the events. To simplify the work, the event queue and component queue share the same data structure. Instead, there are two types of enqueue functions: FIFO enqueue and Priority enqueue. The normal FIFO enqueue is used for the CPU and Disks, while the Priority enqueue is used for the event queue. The priority enqueue will always place the events with smaller time variable above the ones with bigger time variable.

![Readme](https://user-images.githubusercontent.com/28942562/84033213-8321a500-a966-11ea-82ac-61c1b43d12f2.png)


[RUN.pdf](https://github.com/Kylist/C-Projects-in-my-Senior/files/4746053/RUN.pdf)

