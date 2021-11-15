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

enum scheduler_t {
    fcfs=0, 
    rr=1
} scheduler_type;

typedef struct {
    int n;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int *shmPtr;
} Task_param;

void *C1_task(void *task_param) {
    // Parameters Initialization
    Task_param *c1_task_param = (Task_param *) task_param;
    int n1 = c1_task_param->n ;
    int sum;

    // Locking after creation
    pthread_mutex_lock(&c1_task_param->lock);
    while(! c1_task_param->shmPtr[0]) {
        pthread_cond_wait(&c1_task_param->cond, &c1_task_param->lock);
    }
    pthread_mutex_unlock(&c1_task_param->lock);

    int i;
    for(i=0; i<n1; i++) {
        pthread_mutex_lock(&c1_task_param->lock);
        while(! c1_task_param->shmPtr[0]) {
            pthread_cond_wait(&c1_task_param->cond, &c1_task_param->lock);
        }
        sum+=i;
        pthread_mutex_unlock(&c1_task_param->lock);
    }

    pthread_exit((void *)sum);
}

void C1_pipe(int pipefds[2], int sum) {
    close(pipefds[0]);

    char sum_res[100];
    sprintf(sum_res, "%d\n", sum);

    write(pipefds[1], sum_res, strlen(sum_res));

    close(pipefds[1]);
}

void *C2_task(void *task_param) {
    
    // Parameters Initialization
    Task_param *c2_task_param = (Task_param *) task_param;
    int n2 = c2_task_param->n ;

    // Locking after creation
    pthread_mutex_lock(&c2_task_param->lock);
    while(! c2_task_param->shmPtr[1]) {
        pthread_cond_wait(&c2_task_param->cond, &c2_task_param->lock);
    }
    pthread_mutex_unlock(&c2_task_param->lock);
    
    FILE *f;
    f = fopen("./n2.txt", "r");

    if (f==NULL) {
        printf("unable to open");
        fprintf(stderr, "Unable to open the file.\n");
        _exit(EXIT_FAILURE);
    }

    int n,i=0;
    for(; i<n2; i++) {
        pthread_mutex_lock(&c2_task_param->lock);
        while(! c2_task_param->shmPtr[1]) {
            pthread_cond_wait(&c2_task_param->cond, &c2_task_param->lock);
        }
        fscanf(f, "%d\n", &n);
        printf("%d\t%d\n", i, n);        
        pthread_mutex_unlock(&c2_task_param->lock);
    }
    
    fclose(f);

    pthread_exit(EXIT_SUCCESS);
}

void C2_pipe(int pipefds[2]) {
    close(pipefds[0]);
    char *msg = "Done Printing.\n";

    write(pipefds[1], msg, strlen(msg));

    close(pipefds[1]);
}

void *C3_task(void *task_param) {

    // Parameters Initialization
    Task_param *c3_task_param = (Task_param *) task_param;
    int n3 = c3_task_param->n ;
    int sum;

    // Locking after creation
    pthread_mutex_lock(&c3_task_param->lock);
    while(! c3_task_param->shmPtr[2]) {
        pthread_cond_wait(&c3_task_param->cond, &c3_task_param->lock);
    }
    pthread_mutex_unlock(&c3_task_param->lock);
    
    FILE *f;
    f = fopen("./n3.txt", "r");

    if (f==NULL) {
        printf("unable to open");
        fprintf(stderr, "Unable to open the file.\n");
        _exit(EXIT_FAILURE);
    }

    int n,i=0;
    for(; i<n3; i++) {
        pthread_mutex_lock(&c3_task_param->lock);
        while(! c3_task_param->shmPtr[2]) {
            pthread_cond_wait(&c3_task_param->cond, &c3_task_param->lock);
        }
        fscanf(f, "%d\n", &n);
        sum += n;      
        pthread_mutex_unlock(&c3_task_param->lock);
    }
    
    fclose(f);

    pthread_exit((void *)sum);
}

void C3_pipe(int *pipefds, int sum) {
    close(pipefds[0]);

    char sum_res[100];
    sprintf(sum_res, "%d\n", sum);

    write(pipefds[1], sum_res, strlen(sum_res));

    close(pipefds[1]);
}

void c1_process(pthread_t *tid, int n1, int pipefds[2], int shmid) {

    int *shmPtr = shmat(shmid, 0, 0);
    if(shmPtr == (int *)-1) {
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

    while(!shmPtr[3]) {
        
        pthread_mutex_lock(&lock);

        // Critical Section
        if(shmPtr[0]) { pthread_cond_signal(&condition); }
        // End of critical section

        pthread_mutex_unlock(&lock);

    }

    // Controlled by Master 

    void *ret_value;
    int res;
    pthread_join(*tid, &ret_value);
    res = (int*)ret_value;
    
    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    C1_pipe(pipefds, res);

    _exit(EXIT_SUCCESS);
}

void c2_process(pthread_t *tid, int n2, int pipefds[2], int shmid) {
    
    int *shmPtr = shmat(shmid, 0, 0);
    if(shmPtr == (int *)-1) {
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
    while(!shmPtr[4]) {
        
        pthread_mutex_lock(&lock);

        // Critical Section
        if(shmPtr[1]) { pthread_cond_signal(&condition); }
        // End of critical section

        pthread_mutex_unlock(&lock);

    }

    pthread_join(*tid, NULL);

    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    C2_pipe(pipefds);
    _exit(EXIT_SUCCESS);
}

void c3_process(pthread_t *tid, int n3, int pipefds[2], int shmid) {

    int * shmPtr = shmat(shmid, 0, 0);
    if(shmPtr == (int *)-1) {
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

    while(!shmPtr[5]) {
        
        pthread_mutex_lock(&lock);

        // Critical Section
        if(shmPtr[2]) { pthread_cond_signal(&condition); }
        // End of critical section

        pthread_mutex_unlock(&lock);

    }

    void * ret_value;
    pthread_join(*tid, &ret_value);
    int res = (int*)ret_value;

    // destroy condition and mutex
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    C3_pipe(pipefds, res);
    _exit(EXIT_SUCCESS);
}

void *monitor(void* param) {
    // To-do
    int *pipefds = (int *)param;
    char message[30];
    close(pipefds[1]);

    if(read(pipefds[0], message, sizeof(message)) > 0) {
        printf("Master Process : %s\n", message);
    }

    close(pipefds[0]);
    pthread_exit(0);
}


void *m_monitor_thread(void *param) {
    int *pipefds = (int*)param;

    pthread_t monitors[3];

    for(int i=0; i<3; i++) {
        pthread_create(&monitors[i], NULL, monitor, (void *) (pipefds+(2*i)) );
    }
    for(int i=0; i<3; i++) {
        pthread_join(monitors[i], NULL);
    }
    pthread_exit(0);   
}

void scheduler_rr(int * shmPtr) {
      // To-do
}

void scheduler_fcfs(int *shmPtr) {
    // To-do
    

}

int main(int argc, char *argv[]) {
    
    // Initialization
    int pipefds[3][2];
    pid_t pids[3];
    pthread_t tid;

    // Reading command Line arguments
    if(argc!=6) {
        fprintf(stderr, "Please the correct number of arguments.\n");
    }
    scheduler_type = atoi(argv[1]);
    int n[3] = { atoi(argv[2]), atoi(argv[3]), atoi(argv[4]) };

    // Shared Memory
    int shmid;
    int* shmPtr;

    if( (shmid = shmget(2041, sizeof(int) * 6, IPC_CREAT | 0644)) == -1 ) {
        fprintf(stderr, "Failed to create shared memory.");
        exit(EXIT_FAILURE);
    }
    shmPtr = shmat(shmid, 0, 0);
    if(shmPtr == (int *)-1) {
        fprintf(stderr, "Error creating shared memory in parent process.\n");
        exit(EXIT_FAILURE);
    }
    for(int i=0; i<6; i++) {
        shmPtr[i] = 0;
    }

    // Child Process Creation

    for(int i=0; i<3; i++) {
        
        if( ( pipe(pipefds[i]) ) == -1 ) {
            fprintf(stderr, "Initialize Pipe %d for task C%d.\n", i+1, i+1);
            exit(EXIT_FAILURE);
        } 

        pids[i] = fork();
        
        if(pids[i]<0) {
            fprintf(stderr, "Error in creating child Process %d for task C%d.\n", i+1, i+1);
            exit(EXIT_FAILURE);
        } else if (pids[i]==0) {
            switch(i) {
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

    // Invoke the selected scheduler
    if(scheduler_type==rr) {
        scheduler_rr(shmPtr);
    } else {
        scheduler_fcfs(shmPtr);
    }   

    pthread_join(master_monitor, NULL);

    for(int i=0; i<3; i++) {
        wait(NULL);
    }

    // Closing Shared Memory
    if( (shmctl(shmid, IPC_RMID, NULL) ) ==-1) {
        fprintf(stderr, "Error in closing Shared Memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("End Program.\n");
    return 0;

}