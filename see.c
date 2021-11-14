#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<errno.h>

void *C1_task(void *params) {
    printf("C1 task.\n");
}

void *C1_monitor(void *params) {
    printf("C1 monitor.\n");
    // close(pipefd[0][0])
}

void *C2_task(void *params) {
    printf("C2 task.\n");

}

void *C2_monitor(void *params) {
    printf("C2 monitor.\n");
    
}

void *C3_task(void *params) {
    printf("C3 task.\n");
}

void *C3_monitor(void *params) {
    printf("C3 monitor.\n");

}

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;

    if(fork()==0) {
        // M
        printf("Enter Master.\n");
        pthread_t tid[2];
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        
        int pipefd[3][2];
        for(int i=0; i<3; i++) {
            if(pipe(pipefd[i]) == -1) {
                perror("Error in pipe.\n");
                exit(1);
            }
        }

        if(fork()==0) {
            // C1
            printf("Enter C1.\n");
            pthread_create(&tid[0], &attr, C1_task, NULL);
            pthread_create(&tid[1], &attr, C1_monitor, NULL);



        } else {
            if(fork()==0) {
                // C2
                printf("Enter C2.\n");
                pthread_create(&tid[0], &attr, C2_task, NULL);
                pthread_create(&tid[1], &attr, C2_monitor, NULL);


            } else {
                if(fork()==0) {
                    // C3
                    printf("Enter C3.\n");
                    pthread_create(&tid[0], &attr, C3_task, NULL);
                    pthread_create(&tid[1], &attr, C3_monitor, NULL);


                } else {
                    // M
                    wait(&status);
                    wait(&status);
                    wait(&status);
                    
                    
                    printf("Exit Master.\n");
                }
            }
        }
    }
    else {
        wait(NULL);
    }
}