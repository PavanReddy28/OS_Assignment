/*
1. pthread_mutex to stop child task threads - monitor
2. Schedulers - scheduler_rr, scheduler_fcfs
3. Reports
4. Graphs
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

enum scheduler_t
{
    fcfs = 0,
    rr = 1
} scheduler_type;

typedef struct
{
    int n;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int *shmPtr;
} Task_param;

typedef struct
{
    struct timespec scheduler_start;
    struct timespec scheduler_end;

    double burst_time;

} Perf_scheduler;

typedef struct
{
    struct timespec thread_start;
    struct timespec thread_end;

    double burst_time;
} Perf_threads;

typedef struct
{
    struct timespec process_start;
    struct timespec process_end;

    double burst_time;
} Perf_process;

void *C1_task(void *task_param)
{
    // printf("Enter C1 Task Thread....\n");
    // Parameters Initialization
    Task_param *c1_task_param = (Task_param *)task_param;
    int n1 = c1_task_param->n;
    int sum;

    // Locking after creation
    pthread_mutex_lock(&c1_task_param->lock);
    printf("C1 Task Thread - Waiting for cond signal....\n");
    while (!c1_task_param->shmPtr[0])
    {
        pthread_cond_wait(&c1_task_param->cond, &c1_task_param->lock);
    }
    printf("C1 Task Thread - Recieved cond signal....\n");
    pthread_mutex_unlock(&c1_task_param->lock);

    // Performing the task
    int i;
    for (i = 0; i < n1; i++)
    {
        pthread_mutex_lock(&c1_task_param->lock);
        while (!c1_task_param->shmPtr[0])
        {
            pthread_cond_wait(&c1_task_param->cond, &c1_task_param->lock);
        }
        sum += i;
        pthread_mutex_unlock(&c1_task_param->lock);
    }

    // Setting the shared memory to communicate with monitor thread
    pthread_mutex_lock(&c1_task_param->lock);
    c1_task_param->shmPtr[3] = 1;
    pthread_mutex_unlock(&c1_task_param->lock);

    pthread_exit((void *)sum);
}

void C1_pipe(int pipefds[2], int sum)
{
    close(pipefds[0]);

    char sum_res[30];
    sprintf(sum_res, "%d\n", sum);

    write(pipefds[1], sum_res, strlen(sum_res));

    close(pipefds[1]);
}

void *C2_task(void *task_param)
{
    // sleep(1);
    // Parameters Initialization
    Task_param *c2_task_param = (Task_param *)task_param;
    int n2 = c2_task_param->n;

    // Locking after creation
    pthread_mutex_lock(&c2_task_param->lock);
    printf("C2 Task Thread - Waiting for cond signal....\n");
    printf("C2 Task Thread - Running - SHared Memmory Value - %d\n", c2_task_param->shmPtr[1]);
    while (!c2_task_param->shmPtr[1])
    {
        printf("C2 Task Thread - Running - SHared Memmory Value - %d\n", c2_task_param->shmPtr[1]);
        pthread_cond_wait(&c2_task_param->cond, &c2_task_param->lock);
    }
    printf("C2 Task Thread - Recieved cond signal....\n");
    pthread_mutex_unlock(&c2_task_param->lock);

    printf("C2 Task Thread - Reading from file....\n");
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
        pthread_mutex_lock(&c2_task_param->lock);
        while (!c2_task_param->shmPtr[1])
        {
            pthread_cond_wait(&c2_task_param->cond, &c2_task_param->lock);
        }
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

void C2_pipe(int pipefds[2])
{
    close(pipefds[0]);
    char *msg = "Done Printing.\n";

    write(pipefds[1], msg, 30);

    close(pipefds[1]);
}

void *C3_task(void *task_param)
{
    // sleep(2);
    // Parameters Initialization
    Task_param *c3_task_param = (Task_param *)task_param;
    int n3 = c3_task_param->n;
    int sum;

    // Locking after creation
    pthread_mutex_lock(&c3_task_param->lock);
    printf("C3 Task Thread - Waiting for cond signal....\n");
    if (!c3_task_param->shmPtr[2])
    {
        pthread_cond_wait(&c3_task_param->cond, &c3_task_param->lock);
    }
    printf("C3 Task Thread - Recieved cond signal....\n");
    pthread_mutex_unlock(&c3_task_param->lock);

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
        pthread_mutex_lock(&c3_task_param->lock);
        while (!c3_task_param->shmPtr[2])
        {
            pthread_cond_wait(&c3_task_param->cond, &c3_task_param->lock);
        }
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

void C3_pipe(int *pipefds, int sum)
{
    close(pipefds[0]);

    char sum_res[100];
    sprintf(sum_res, "%d\n", sum);

    write(pipefds[1], sum_res, 30);

    close(pipefds[1]);
}

void c1_process(pthread_t *tid, int n1, int pipefds[2], int shmid)
{

    int *shmPtr = shmat(shmid, 0, 0);
    if (shmPtr == (int *)-1)
    {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        _exit(EXIT_FAILURE);
    }

    pthread_mutex_t lock;
    pthread_cond_t condition;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);

    // Thread Parameters
    Task_param c_task = {
        .cond = condition,
        .lock = lock,
        .n = n1,
        .shmPtr = shmPtr,
    };

    pthread_create(tid, NULL, &C1_task, (void *)&c_task);
    printf("C1 Task Thread - After thread creation.\n");
    while (!shmPtr[3])
    {

        pthread_mutex_lock(&lock);

        // Critical Section
        if (shmPtr[0])
        {
            pthread_cond_signal(&condition);
        }
        // End of critical section

        pthread_mutex_unlock(&lock);
    }

    // Controlled by Master

    void *ret_value;
    int res;
    pthread_join(*tid, &ret_value);
    res = (int *)ret_value;

    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    C1_pipe(pipefds, res);

    _exit(EXIT_SUCCESS);
}

void c2_process(pthread_t *tid, int n2, int pipefds[2], int shmid)
{

    int *shmPtr = shmat(shmid, 0, 0);
    if (shmPtr == (int *)-1)
    {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        _exit(EXIT_FAILURE);
    }

    pthread_mutex_t lock;
    pthread_cond_t condition;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);

    // Thread Parameters
    Task_param c_task = {
        .cond = condition,
        .lock = lock,
        .n = n2,
        .shmPtr = shmPtr,
    };

    pthread_create(tid, NULL, &C2_task, (void *)&c_task);
    printf("C2 Task Thread - After thread creation. Shared Memory value %d.\n", shmPtr[4]);
    while (!shmPtr[4])
    {
        pthread_mutex_lock(&lock);
        // Critical Section
        if (shmPtr[1])
        {
            pthread_cond_signal(&condition);
        }
        // End of critical section
        pthread_mutex_unlock(&lock);
    }

    printf("Exit Loop in C2 Process.\n");

    pthread_join(*tid, NULL);

    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    C2_pipe(pipefds);

    // shmdt((void *)shmPtr);
    _exit(EXIT_SUCCESS);
}

void c3_process(pthread_t *tid, int n3, int pipefds[2], int shmid)
{

    int *shmPtr = shmat(shmid, 0, 0);
    if (shmPtr == (int *)-1)
    {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        _exit(EXIT_FAILURE);
    }

    pthread_mutex_t lock;
    pthread_cond_t condition;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);

    // Thread Parameters
    Task_param c_task = {
        .cond = condition,
        .lock = lock,
        .n = n3,
        .shmPtr = shmPtr,
    };

    pthread_create(tid, NULL, &C3_task, (void *)&c_task);
    printf("C3 Task Thread - After thread creation.\n");
    while (!shmPtr[5])
    {

        pthread_mutex_lock(&lock);

        // Critical Section
        if (shmPtr[2])
        {
            pthread_cond_signal(&condition);
        }
        // End of critical section

        pthread_mutex_unlock(&lock);
    }

    void *ret_value;
    pthread_join(*tid, &ret_value);
    int res = (int *)ret_value;

    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    C3_pipe(pipefds, res);

    _exit(EXIT_SUCCESS);
}

void *monitor(void *param)
{
    // To-do
    int *pipefds = (int *)param;
    char message[30];
    close(pipefds[1]);

    if (read(pipefds[0], message, sizeof(message)) > 0)
    {
        printf("Master Process : %s", message);
    }

    close(pipefds[0]);
    pthread_exit(0);
}

void *m_monitor_thread(void *param)
{
    int *pipefds = (int *)param;

    pthread_t monitors[3];

    for (int i = 0; i < 3; i++)
    {
        pthread_create(&monitors[i], NULL, monitor, (void *)(pipefds + (2 * i)));
    }
    for (int i = 0; i < 3; i++)
    {
        pthread_join(monitors[i], NULL);
    }
    pthread_exit(0);
}

void scheduler_rr(int *shmPtr, int time_quantum)
{
    // To-do
    printf("RR Start...\n");
    int done = 0;

    int proc_start[3] = {0, 0, 0};
    int proc_mark[3] = {0, 0, 0};

    Perf_scheduler *perf_scheduler;
    Perf_process *perf_process[3];

    printf("Get scheduler start time.\n");
    //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &perf_scheduler->scheduler_start);

    while (!done)
    {
        done = 1;
        for (int i = 0; i < 3; i++)
        {
            if (!proc_start[i])
            {
                printf("Running Child Process %d...\n", i + 1);
                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i]->process_start);
                proc_start[i] = 1;
            }

            if (!shmPtr[i + 3])
            {
                done = 0;
                struct timespec quantumStart, quantumEnd;

                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &quantumStart);
                double elapsedTime = 0;

                shmPtr[i] = 1;

                while (!shmPtr[i + 3] && elapsedTime < time_quantum)
                {
                    // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &quantumEnd);
                    // //elapsedTime = getElapsedTime(quantumStart, quantumEnd);
                    // elapsedTime = (quantumEnd.tv_sec - quantumStart.tv_sec) * 1000000.0;
                    // elapsedTime += (quantumEnd.tv_sec - quantumStart.tv_sec) / 1000.0;
                    continue;
                }
                shmPtr[i] = 0;

                //perf_process[i]->burst_time += elapsedTime;
            }

            if (shmPtr[i + 3] && !proc_mark[i])
            {
                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i]->process_end);
                proc_mark[i] = 1;
                printf("End Child Process %d...\n", i + 1);
            }
        }
    }
    //printf("Get scheduler end time.\n");
    //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &perf_scheduler->scheduler_end);

    //for(int i=0;i<3;i++){
    //    perf_scheduler[i]->
    //}
}

void scheduler_fcfs(int *shmPtr)
{
    printf("FCFS Start...\n");
    // Initialization
    Perf_scheduler *perf_scheduler;
    Perf_process *perf_process[3];

    printf("Get scheduler start time.\n");
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &perf_scheduler->scheduler_start);

    for (int i = 0; i < 3; i++)
    {
        printf("Running Child Process %d...\n", i + 1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i]->process_start);

        // Running the task
        shmPtr[i] = 1;
        while (!shmPtr[i + 3])
        {
            continue;
        }
        shmPtr[i] = 0;

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &perf_process[i]->process_end);
        printf("End Child Process %d...\n", i + 1);

        // Burst time in Microseconds
        // perf_process[i]->burst_time = (perf_process[i]->process_end.tv_sec - perf_process[i]->process_start.tv_sec) * 1e6 + (perf_process[i]->process_end.tv_nsec - perf_process[i]->process_start.tv_nsec) / 1e3;
        // printf("Process %d Times:\n\tBurst Time = %lf\n", i + 1, perf_process[i]->burst_time);
    }

    printf("Get scheduler end time.\n");
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &perf_scheduler->scheduler_end);

    // printf(".\n");
    // perf_scheduler->burst_time = (perf_scheduler->scheduler_end.tv_sec - perf_scheduler->scheduler_start.tv_sec) * 1e6 + (perf_scheduler->scheduler_end.tv_nsec - perf_scheduler->scheduler_start.tv_nsec) / 1e3;
    // printf("Scheduler Times:\n\tBurst Time = %lf\n", perf_scheduler->burst_time);
}

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
    scheduler_type = atoi(argv[1]);
    int n[3] = {atoi(argv[2]), atoi(argv[3]), atoi(argv[4])};
    int time_quantum = atoi(argv[5]);

    // Shared Memory
    int shmid;
    int *shmPtr;

    if ((shmid = shmget(2041, sizeof(int) * 6, IPC_CREAT | 0644)) == -1)
    {
        fprintf(stderr, "Failed to create shared memory.");
        exit(EXIT_FAILURE);
    }
    shmPtr = shmat(shmid, 0, 0);
    if (shmPtr == (int *)-1)
    {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 6; i++)
    {
        shmPtr[i] = 0;
    }

    // Child Process Creation
    printf("Children Process Creation started.\n");
    for (int i = 0; i < 3; i++)
    {

        if ((pipe(pipefds[i])) == -1)
        {
            fprintf(stderr, "Initialize Pipe %d for task C%d.\n", i + 1, i + 1);
            exit(EXIT_FAILURE);
        }

        pids[i] = fork();

        if (pids[i] < 0)
        {
            fprintf(stderr, "Error in creating child Process %d for task C%d.\n", i + 1, i + 1);
            exit(EXIT_FAILURE);
        }
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
        printf("Round robin scheduler selected.\n");
        scheduler_rr(shmPtr, time_quantum);
    }
    else
    {
        printf("FCFS scheduler selected.\n");
        scheduler_fcfs(shmPtr);
    }

    printf("Scheduler end.\n");

    pthread_join(master_monitor, NULL);

    for (int i = 0; i < 3; i++)
    {
        wait(NULL);
    }

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