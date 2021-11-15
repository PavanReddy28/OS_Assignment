/*
Group Members:
1.  2019A7PS0038H			Pavan Kumar Reddy Yannam
2.  2019A7PS0043H			Vineet Venkatesh
3.  2019A8PS1357H			Avinash Gondela
4.  2019AAPS0241H			Akhilesh Gowrishetty
5.  2019A3PS1323H			Anantha Sai Satwik Vysyaraju
6.  2019A8PS1275H			Bhargava Teja Uppuluri
7.  2019A7PS0017H			Bokkasam Venkata Sai Ruthvik
8.  2019A8PS0651H			Divakarla Vamsi Krishna
*/

// Required libraries.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/shm.h>    
#include <sys/types.h>
#include <sys/wait.h>   
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

// Enumeration for scheduler choice.
enum scheduler_t
{
    fcfs = 0,
    rr = 1
} scheduler_type;

// Structure to store parameters for passing to task threads.
typedef struct
{
    int n;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int *shmPtr;
} Task_param;

// Structure to store performance parameters of scheduler.
typedef struct
{
    struct timespec scheduler_start;
    struct timespec scheduler_end;

    double burst_time;

} Perf_scheduler;

// Structure to store performance parameters of process.
typedef struct
{
    struct timespec process_start;
    struct timespec process_end;

    double burst_time;
} Perf_process;

// Function for the task 1 thread.
void *C1_task(void *task_param)
{
    
    // Parameters Initialization
    Task_param *c1_task_param = (Task_param *)task_param;
    int n1 = c1_task_param->n;
    u_int64_t sum;

    // Stops the running of the thread and waits for process.
    while (!c1_task_param->shmPtr[6])
    {
        continue;
    }
    
    // Locking after creation
    pthread_mutex_lock(&c1_task_param->lock);
    // printf("C1 Task Thread - Waiting for cond signal....\n");
    while (!c1_task_param->shmPtr[0]);
    // printf("C1 Task Thread - Recieved cond signal....\n");
    pthread_mutex_unlock(&c1_task_param->lock);

    // Performing the task
    int i;
    for (i = 0; i < n1; i++)
    {
        pthread_mutex_lock(&c1_task_param->lock);
        while (!c1_task_param->shmPtr[0]);
        // printf("%d\n", i);
        sum += i;
        pthread_mutex_unlock(&c1_task_param->lock);
    }

    // Setting the shared memory to communicate with monitor thread
    pthread_mutex_lock(&c1_task_param->lock);
    c1_task_param->shmPtr[3] = 1;
    pthread_mutex_unlock(&c1_task_param->lock);

    pthread_exit((void *)sum);
}

// Pipe function to send the computed data in the task 1 thread.
void C1_pipe(int pipefds[2], u_int64_t sum)
{
    // Close the read end.
    close(pipefds[0]);

    char sum_res[30];
    sprintf(sum_res, "%" PRIu64"\n", sum);
    // Write to pipe.
    write(pipefds[1], sum_res, strlen(sum_res));
    // Close the write end.
    close(pipefds[1]);
}

// Function for the task 2 thread.
void *C2_task(void *task_param)
{
    // Parameters Initialization
    Task_param *c2_task_param = (Task_param *)task_param;
    int n2 = c2_task_param->n;

    while (!c2_task_param->shmPtr[7]);

    // Locking after creation
    pthread_mutex_lock(&c2_task_param->lock);
    // printf("C2 Task Thread - Waiting for cond signal....\n");
    while (!c2_task_param->shmPtr[1]);
    // printf("C2 Task Thread - Recieved cond signal....\n");
    pthread_mutex_unlock(&c2_task_param->lock);

    // printf("C2 Task Thread - Reading from file....\n");
    FILE *f;
    f = fopen("./n2.txt", "r");

    if (f == NULL)
    {
        printf("unable to open");
        fprintf(stderr, "Unable to open the file.\n");
        _exit(EXIT_FAILURE);
    }

    int n, i = 0;
    for (; i < n2; i++)
    {   
        // Scanning and printing to console.
        pthread_mutex_lock(&c2_task_param->lock);
        while (!c2_task_param->shmPtr[1]);
        fscanf(f, "%d\n", &n);
        printf("%d\t%d\n", i, n);
        pthread_mutex_unlock(&c2_task_param->lock);
    }

    fclose(f);

    // Setting the shared memory to communicate with monitor thread
    pthread_mutex_lock(&c2_task_param->lock);
    c2_task_param->shmPtr[4] = 1;
    pthread_mutex_unlock(&c2_task_param->lock);

    pthread_exit(EXIT_SUCCESS);
}

// Pipe function to send the computed data in the task 2 thread.
void C2_pipe(int pipefds[2])
{
    // Close the read end.
    close(pipefds[0]);
    char *msg = "Done Printing.\n";
    // Write to pipe.
    write(pipefds[1], msg, 30);
    // Close the write end.
    close(pipefds[1]);
}

// Function for the task 3 thread.
void *C3_task(void *task_param)
{
    // Parameters Initialization
    Task_param *c3_task_param = (Task_param *)task_param;
    int n3 = c3_task_param->n;
    u_int64_t sum;

    while (!c3_task_param->shmPtr[8]);

    // Locking after creation
    pthread_mutex_lock(&c3_task_param->lock);
    // printf("C3 Task Thread - Waiting for cond signal....\n");
    while (!c3_task_param->shmPtr[2]);
    // printf("C3 Task Thread - Recieved cond signal....\n");
    pthread_mutex_unlock(&c3_task_param->lock);

    // printf("C3 Task Thread - Reading from file....\n");
    FILE *f;
    f = fopen("./n3.txt", "r");

    if (f == NULL)
    {
        fprintf(stderr, "Unable to open the file.\n");
        _exit(EXIT_FAILURE);
    }

    int n, i = 0;
    for (; i < n3; i++)
    {   
        // Scanning and comptuting the sum.
        pthread_mutex_lock(&c3_task_param->lock);
        while (!c3_task_param->shmPtr[2]);
        fscanf(f, "%d\n", &n);
        sum += n;
        pthread_mutex_unlock(&c3_task_param->lock);
    }

    fclose(f);

    // Setting the shared memory to communicate with monitor thread
    pthread_mutex_lock(&c3_task_param->lock);
    c3_task_param->shmPtr[5] = 1;
    pthread_mutex_unlock(&c3_task_param->lock);

    pthread_exit((void *)sum);
}

// Pipe function to send the computed data in the task 3 thread.
void C3_pipe(int *pipefds, uint64_t sum)
{
    // Close the read end.
    close(pipefds[0]);

    char sum_res[100];
    sprintf(sum_res, "%" PRIu64 "\n", sum);
    // Write to pipe.
    write(pipefds[1], sum_res, 30);
    // Close the write end.
    close(pipefds[1]);
}

// Child process: C1
void c1_process(pthread_t *tid, int n1, int pipefds[2], int shmid)
{
    // Initializing the variables.
    int *shmPtr = shmat(shmid, 0, 0);
    if (shmPtr == (int *)-1)
    {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        _exit(EXIT_FAILURE);
    }
    // Creating the mutex and conditions.
    pthread_mutex_t lock;
    pthread_cond_t condition;
    // Initializing the mutex and conditions.
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);

    // Thread Parameters
    Task_param c_task = {
        .cond = condition,
        .lock = lock,
        .n = n1,
        .shmPtr = shmPtr,
    };

    // Creating the Task thread.
    pthread_create(tid, NULL, &C1_task, (void *)&c_task);

    // While the process is not finished:
    while (!shmPtr[3])
    {
        // Locking the mutex
        pthread_mutex_lock(&lock);

        // Critical Section
        if (shmPtr[0])
        {
            pthread_cond_signal(&condition);
        }
        // End of critical section
        // Unlocking the mutex.
        pthread_mutex_unlock(&lock);
    }

    // Controlled by Master

    void *ret_value;
    uint64_t res;
    // Waiting for the task thread to join.
    pthread_join(*tid, &ret_value);
    res = (uint64_t *)ret_value;

    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    // Calling the pipe function.
    C1_pipe(pipefds, res);

    _exit(EXIT_SUCCESS);
}

// Child process: C2
void c2_process(pthread_t *tid, int n2, int pipefds[2], int shmid)
{
    // Initializing the variables.
    int *shmPtr = shmat(shmid, 0, 0);
    if (shmPtr == (int *)-1)
    {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        _exit(EXIT_FAILURE);
    }
    // Creating the mutex and conditions.
    pthread_mutex_t lock;
    pthread_cond_t condition;
    // Initializing the mutex and conditions.
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);

    // Thread Parameters
    Task_param c_task = {
        .cond = condition,
        .lock = lock,
        .n = n2,
        .shmPtr = shmPtr,
    };
    // Creating the Task thread.
    pthread_create(tid, NULL, &C2_task, (void *)&c_task);

    // While the process is not finished:
    while (!shmPtr[4])
    {
        // Locking the mutex
        pthread_mutex_lock(&lock);

        // Critical Section
        if (shmPtr[1])
        {
            pthread_cond_signal(&condition);
        }
        // End of critical section
        // Unlocking the mutex.
        pthread_mutex_unlock(&lock);
    }
    // Waiting for the task thread to join.
    pthread_join(*tid, NULL);

    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    // Calling the pipe function.
    C2_pipe(pipefds);

    _exit(EXIT_SUCCESS);
}

// Child process: C3
void c3_process(pthread_t *tid, int n3, int pipefds[2], int shmid)
{
    // Initializing the variables.
    int *shmPtr = shmat(shmid, 0, 0);
    if (shmPtr == (int *)-1)
    {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        _exit(EXIT_FAILURE);
    }
    // Creating the mutex and conditions.
    pthread_mutex_t lock;
    pthread_cond_t condition;
    // Initializing the mutex and conditions.
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);

    // Thread Parameters
    Task_param c_task = {
        .cond = condition,
        .lock = lock,
        .n = n3,
        .shmPtr = shmPtr,
    };
    // Creating the Task thread.
    pthread_create(tid, NULL, &C3_task, (void *)&c_task);

    // While the process is not finished:
    while (!shmPtr[5])
    {
        // Locking the mutex
        pthread_mutex_lock(&lock);

        // Critical Section
        if (shmPtr[2])
        {
            pthread_cond_signal(&condition);
        }
        // End of critical section
        // Unlocking the mutex.
        pthread_mutex_unlock(&lock);
    }

    // Controlled by master.

    void *ret_value;
    // Waiting for the task thread to join.
    pthread_join(*tid, &ret_value);
    uint64_t res = (uint64_t *)ret_value;

    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    // Calling the pipe function.
    C3_pipe(pipefds, res);

    _exit(EXIT_SUCCESS);
}

// Monitor threads for communicating to the child processes.
void *monitor(void *param)
{
    // Initializing the variables.
    int *pipefds = (int *)param;
    char message[30];
    // Close the write end.
    close(pipefds[1]);

    // Wait for the message from the process.
    if (read(pipefds[0], message, sizeof(message)) > 0)
    {   
        // Print the message to the console.
        printf("Master Process : %s", message);
    }
    // Close the read end.
    close(pipefds[0]);

    pthread_exit(0);
}

// Thread to create the monitor threads that communicate with the processes.
void *m_monitor_thread(void *param)
{
    // Initializing the variables.
    int *pipefds = (int *)param;
    pthread_t monitors[3];

    for (int i = 0; i < 3; i++)
    {   
        // Creation of threads.
        pthread_create(&monitors[i], NULL, monitor, (void *)(pipefds + (2 * i)));
    }
    for (int i = 0; i < 3; i++)
    {
        // Wait for the threads to join.
        pthread_join(monitors[i], NULL);
    }
    pthread_exit(0);
}

// This function to calculate the difference in time (in microseconds).
double diff_time(struct timespec t2, struct timespec t1) {
    return (t2.tv_sec - t1.tv_sec) * 1e6 + (t2.tv_nsec - t1.tv_nsec) / 1e3;
}

// Function apppends to a csv file and stores the performance data. 
void report_generator(Perf_scheduler perf_scheduler, Perf_process *perf_process, int *n) {
    // Print the total time in console.
    printf("Total time taken: %lf.\n", diff_time(perf_scheduler.scheduler_end, perf_scheduler.scheduler_start));
    // process, n, bt, tat, wt
    FILE *f;
    f = fopen("reports_rr.csv", "a+");
    // Print to console and append to file.
    for(int i=0; i<3; i++) {
        double arrival, tat, wt;
        printf("Process %d:\n", i+1);
        printf("Burst Time = %lf\n", perf_process[i].burst_time);
        arrival = diff_time(perf_process[i].process_start, perf_scheduler.scheduler_start);
        tat = diff_time(perf_process[i].process_end, perf_scheduler.scheduler_start);
        wt = tat - perf_process[i].burst_time;
        printf("Process Scheduled time Time = %lf\n", arrival);
        printf("Turn Around Time = %lf\n", tat);
        printf("Wait Time = %lf\n", wt);

        char output[100];
        sprintf(output, "%d,%d,%lf,%lf,%lf,%lf\n", i+1, n[i], perf_process[i].burst_time, tat, wt, perf_scheduler.burst_time);
        fprintf(f, output, strlen(output));
    }
    fclose(f);
}

// Round Robin Scheduling Algorithm.
void scheduler_rr(int *shmPtr, double time_quantum, int *n)
{
    // Initialization of variables.
    printf("RR Start...\n");
    // When all processes are finished, done is set to 1.
    int done = 0;

    // Array to store if the process has started.
    int proc_start[3] = {0, 0, 0};
    // Array to store if the end time of the process has been saved.
    int proc_mark[3] = {0, 0, 0};

    // Structures.
    Perf_scheduler perf_scheduler;
    Perf_process perf_process[3];
    struct timespec start, end;

    printf("Get scheduler start time.\n");
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &perf_scheduler.scheduler_start);

    // While all processes are not finished.
    while (!done)
    {
        done = 1;

        // Loop through the processes in round robin fashion.
        for (int i = 0; i < 3; i++)
        {
            
            // If the process has not started.
            if (!proc_start[i])
            {
                printf("Running Child Process %d...\n", i + 1);
                // Allowing the task thread to continue after creation.
                shmPtr[i + 6] = 1;
                // Storing the start time.
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i].process_start);
                printf("Start time %ld\n", start.tv_nsec);
                proc_start[i] = 1;
            }

            // If the process has not finished.
            if (!shmPtr[i + 3])
            {   
                // Set process to running.                
                shmPtr[i] = 1;
                printf("R%d...\n", i + 1);
                done = 0;
                struct timespec quantumStart, quantumEnd;

                double elapsedTime = 0;
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &quantumStart);

                // While the process is not finished and the time quantum has not finished.
                while (!shmPtr[i + 3] && elapsedTime < time_quantum)
                {
                    // printf("Enter loop - Child Process %d...\n", i + 1);
                    shmPtr[i] = 1;
                    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &quantumEnd);
                    elapsedTime = diff_time(quantumEnd, quantumStart);
                    continue;
                }
                // Set process to not running.
                shmPtr[i] = 0;
                
                // Add the elapsed time to burst time.
                perf_process[i].burst_time += elapsedTime;
            }

            // If the process has finished but the end time is not saved.
            if (shmPtr[i + 3] && !proc_mark[i])
            {
                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i]->process_end);
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i].process_end);
                // Marking the process as end time saved.
                proc_mark[i] = 1;
                // printf("End Child Process %d...\n", i + 1);
            }
        }
    }
    printf("Get scheduler end time.\n");
    // Store the end time of the scheduler.
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &perf_scheduler.scheduler_end);
    perf_scheduler.burst_time = time_quantum;
    report_generator(perf_scheduler, perf_process, n);
}

// First Come First Serve Scheduling Algorithm.
void scheduler_fcfs(int *shmPtr, int* n)
{
    printf("FCFS Start...\n");
    // Initialization
    Perf_scheduler perf_scheduler;
    Perf_process perf_process[3];

    printf("Get scheduler start time.\n");
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &perf_scheduler.scheduler_start);

    // Looping the process one by one.
    for (int i = 0; i < 3; i++)
    {
        // printf("Running Child Process %d...\n", i + 1);

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i].process_start);
        // Allowing the task thread to continue after creation.
        shmPtr[i + 6] = 1;
        // Set process to running.
        shmPtr[i] = 1;
        while (!shmPtr[i + 3]);
        // Set process to not running.
        shmPtr[i] = 0;
        // Store the process end time.
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i].process_end);
        
        printf("End Child Process %d...\n", i + 1);
        // Set the process burst time.
        perf_process[i].burst_time = diff_time(perf_process[i].process_end, perf_process[i].process_start);
    }

    printf("Get scheduler end time.\n");
    // Store the end time of the scheduler.
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &perf_scheduler.scheduler_end);
    perf_scheduler.burst_time = diff_time(perf_scheduler.scheduler_end, perf_scheduler.scheduler_start);
    report_generator(perf_scheduler, perf_process, n);
}

// Main function.
int main(int argc, char *argv[])
{
    // Initialization
    int pipefds[3][2];
    pid_t pids[3];
    pthread_t tid;

    // Reading command Line arguments
    if (argc != 6)
    {
        fprintf(stderr, "Please the correct number of arguments.\n");
    }
    // Assigning the command line arguments.
    scheduler_type = atoi(argv[1]);
    int n[3] = {atoi(argv[2]), atoi(argv[3]), atoi(argv[4])};
    double time_quantum = atof(argv[5]);

    // Shared Memory
    int shmid;
    int *shmPtr;
    // Initialization of shared memory.
    if ((shmid = shmget(2041, sizeof(int) * 9, IPC_CREAT | 0644)) == -1)
    {
        fprintf(stderr, "Failed to create shared memory.");
        exit(EXIT_FAILURE);
    }
    // Attaching the shared memory.
    shmPtr = shmat(shmid, 0, 0);
    if (shmPtr == (int *)-1)
    {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        exit(EXIT_FAILURE);
    }
    // Clearing the shared memory.
    for (int i = 0; i < 9; i++)
    {
        shmPtr[i] = 0;
    }

    // Child Process Creation
    printf("Children Process Creation started.\n");
    for (int i = 0; i < 3; i++)
    {
        // Creating the pipes.
        if ((pipe(pipefds[i])) == -1)
        {
            fprintf(stderr, "Initialize Pipe %d for task C%d.\n", i + 1, i + 1);
            exit(EXIT_FAILURE);
        }

        // Forking the child processes.
        pids[i] = fork();

        if (pids[i] < 0)
        {
            fprintf(stderr, "Error in creating child Process %d for task C%d.\n", i + 1, i + 1);
            exit(EXIT_FAILURE);
        }
        // Child processes.
        else if (pids[i] == 0)
        {
            switch (i)
            {
            case 0:
                c1_process(&tid, n[0], pipefds[0], shmid);
                break;
            case 1:
                c2_process(&tid, n[1], pipefds[1], shmid);
                break;
            case 2:
                c3_process(&tid, n[2], pipefds[2], shmid);
                break;
            default:
                break;
            }
        }
    }

    // Create Monitor thread from Master Process
    pthread_t master_monitor;
    pthread_create(&master_monitor, NULL, m_monitor_thread, (void *)pipefds);

    printf("Scheduler start.\n");

    // Invoke the selected scheduler
    if (scheduler_type == rr)
    {
        printf("Round Robin Scheduler selected.\n");
        scheduler_rr(shmPtr, time_quantum, n);
    }
    else
    {
        printf("FCFS Scheduler selected.\n");
        scheduler_fcfs(shmPtr, n);
    }

    printf("Scheduler end.\n");

    // Waiting for the master_monitor thread to join.
    pthread_join(master_monitor, NULL);

    // Wait for the child processes to exit.
    for (int i = 0; i < 3; i++)
    {
        wait(NULL);
    }

    // Detach the shared memory.
    shmdt((void *)shmPtr);
    // Closing Shared Memory
    if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
    {
        fprintf(stderr, "Error in closing Shared Memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("End Program.\n");
    return 0;
}