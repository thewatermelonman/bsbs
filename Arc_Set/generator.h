#ifndef GENERATOR_H
#define GENERATOR_H


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>



#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>

#define BUFFER "/buffer"
#define SEM_FREE "/free"
#define SEM_USED "/used"
#define SEM_MUTEX "/mutex"


struct edge {
    int a;
    int b;
};

struct arc_set{
    int len;
    struct edge * edges;
};

struct buffer {
    int write;
    int read;
    int len;
    bool terminate;
    struct arc_set array[8];
};

#endif // GENERATOR_H