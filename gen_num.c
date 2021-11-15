// C program to generate the files for C2 and C3 tasks.

// Required libraries.
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>

// Main function.
int main()
{
    // Creating the file descriptor.
    FILE *p;
    // Opening the file.
    // For C2 task.
    p = fopen("./n2.txt", "a+");
    int num = 1000100;
    for (int i = 0; i < num; i++)
    {
        // Writing random numbers to file.
        fprintf(p, "%d\n", rand() % 1000000);
    }
    // Closing the file.
    fclose(p);
    // Creating the file descriptor.
    FILE *f;
    // Opening the file.
    // For C3 task.
    f = fopen("./n3.txt", "a+");
    for (int i = 0; i < num; i++)
    {
        // Writing random numbers to file.
        fprintf(f, "%d\n", rand() % 1000000);
    }
    // Closing the file.
    fclose(f);
    return 0;
}