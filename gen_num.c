#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{

    FILE *p;
    p = fopen("./n2.txt", "a+");
    int num = 1000100;
    for (int i = 0; i < num; i++)
    {
        fprintf(p, "%d\n", rand() % 1000000);
    }
    fclose(p);

    FILE *f;
    f = fopen("./n3.txt", "a+");
    for (int i = 0; i < num; i++)
    {
        fprintf(f, "%d\n", rand() % 1000000);
    }
    fclose(f);
    return 0;
}