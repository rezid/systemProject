#include "conduct.h"
/* Pour les constantes EXIT_SUCCESS et EXIT_FAILURE */
#include <stdlib.h>
/* Pour fprintf() */
#include <stdio.h>
/* Pour fork() */
#include <unistd.h>
/* Pour perror() et errno */
#include <errno.h>
/* Pour le type pid_t (sudo apt-get install libc6-dev-i386) */
#include <sys/types.h>

struct point {
    int x;
    int y;
};

int main(void)
{
    struct conduct * c;

    // Create a 64 point conduct
    //c = conduct_create("temp", sizeof(struct conduct), 64 * sizeof(struct conduct));
    c = conduct_open("temp");

    // Write the first point to consduct
    struct point point_2d = {.x = 5, .y = 3};
    ssize_t result = conduct_write(c, &point_2d, sizeof(point_2d));

    // Read the first point to consduct
    //struct point point_2d;
    //ssize_t result = conduct_read(c, &point_2d, sizeof(point_2d));

    // print result
    //printf("result is %d\n", result);

    printf("result is %d %d\n", point_2d.x, point_2d.y);
    return EXIT_SUCCESS;
}
