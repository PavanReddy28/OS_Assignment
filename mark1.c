#include<stdio.h>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>

void enqueue()
{

}
void dequeue()
{

}

void *C1_task(void* n1) 
{
    uint32_t sum=0;
    int loop_variable;
    for(loop_variable=0; loop_variable<atoi(n1); loop_variable++)
    {
        sum+=rand();
    }
    printf("C1: %d\n",sum);
}

void *C2_task(void* n2)
{
    FILE* numbers;
    numbers = fopen("n2.txt", "w+");
    int n2_clone=atoi(n2);

    

    while (n2_clone--)
    {
        fprintf(numbers,"%d\n",123);
        
    }
    n2_clone = atoi(n2);
    while(n2_clone--)
    {
        int num=getw(numbers);
        printf(" C2: %d\n",num);
    }
    fclose(numbers);

    FILE *numbe = fopen("n2.txt", "r");
    int k;
    while(k=getw(numbe) != EOF){
        printf("%d\n", k);
    }
    fclose(numbe);
    printf("Done Printing\n");
}

void *C3_task(void* n3) {
    FILE* numbers;
    numbers = fopen("n3.txt", "w+");
    int n3_clone=atoi(n3);
    while (n3_clone--)
    {
        fprintf(numbers,"%d\n",rand());
    }
    uint32_t sum=0;
    n3_clone = atoi(n3);
    while(n3_clone--)
    {
        int num=getw(numbers);
        sum+=num;
    }
    fclose(numbers);
    printf("C3: %d\n",sum);
    
}

void *C1_monitor(void* params) {
    
}

void *C2_monitor(void* params) {
    
}

void *C3_monitor(void* params) {
    
}

int main(int argc, char *argv[])
{
     int scheduler = atoi(argv[1]);
     int n1 = atoi(argv[2]), n2 = atoi(argv[3]), n3 = atoi(argv[4]);
     int time_quantum = atoi(argv[5]);

    pid_t pid[3];

    int i;
    for (i=0; i<3; i++) {
        pid[i] = fork();

        if (pid[i] < 0) {
            fprintf(stderr, "Fork failed");
            return 1;
        }
        
        if(pid[i] == 0) {
            pthread_t tid[2];
            pthread_attr_t attr[2];

            pthread_attr_init(&attr[0]);
            pthread_attr_init(&attr[1]);

            switch (i) {
                case 0:
                pthread_create(&tid[0], &attr[0], C1_task, &n1);
                pthread_create(&tid[1], &attr[1], C1_monitor, NULL);
                break;

                case 1:
                pthread_create(&tid[0], &attr[0], C2_task, &n2);
                pthread_create(&tid[1], &attr[1], C2_monitor, NULL);
                break;

                case 2:
                pthread_create(&tid[0], &attr[0], C3_task, &n3);
                pthread_create(&tid[1], &attr[1], C3_monitor, NULL);
                break;

                default:
                printf("Does not exist");
            }
            
            pthread_join(tid[0], NULL);
            pthread_join(tid[1], NULL);
        }
    } 
    
}