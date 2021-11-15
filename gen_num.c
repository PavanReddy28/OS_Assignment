#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>
#include<sys/wait.h>


int main() {
    

    FILE *p;
    p = fopen("./n2.txt", "a+");

    int num = 1000100;
    //fprintf(p, "%d", 123);
    for (int i=0;i<num;i++) {
        //fscanf(p, "%d\n", &n);
        //putw(rand(), p);
        fprintf(p, "%d\n", rand()%1000000);
        //i = getw(p);
        // printf("%d\n", n);
    }
    fclose(p);

    FILE *f;
    f = fopen("./n3.txt", "a+");

    // int num = 1000100;
    //fprintf(p, "%d", 123);
    for (int i=0;i<num;i++) {
        //fscanf(p, "%d\n", &n);
        //putw(rand(), p);
        fprintf(f, "%d\n", rand()%1000000);
        //i = getw(p);
        // printf("%d\n", n);
    }
    fclose(f);
    return 0;
}