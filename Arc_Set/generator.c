#include "generator.h"


static void readInput(int, char **, struct edge *);
static int graphLen(struct edge *, int);
static void randomGraphList(int [] , int);
static int getIndexInPermutation(int *, int , int );
static bool toBeSelected(struct edge, int *, int);
static int arcSetSize(struct edge * , int, int *, int);
static void arcSet(struct edge *, int, int *, int, struct edge *);
void outputSet(struct edge *, int);
int arcSetAlgo(int * graph, int graph_len, struct edge * edges, int edges_len);
static void init(void);
static void write_to_buffer(struct arc_set);
static char * PROG_NAME;
struct buffer * buff = NULL;

//semaphores
static sem_t s_read = NULL;
static sem_t s_mutex = NULL;

int main(int argc, char *argv[]) {
    srand(((unsigned int) time(NULL)));
    PROG_NAME = *argv;
    init();
    if (argc < 2) {
        return EXIT_FAILURE;
    }
    int edges_len = argc - 1;
    struct edge edges[edges_len];
    readInput(edges_len, argv, edges);
    int graph_len = graphLen(edges, edges_len);
    int graph[graph_len];
    for (int i = 0; i < graph_len; ++i) {
        graph[i] = i;
    }
    randomGraphList(graph, graph_len);
    while (arcSetAlgo(graph, graph_len, edges, edges_len) != 0);
    return EXIT_SUCCESS;
}

int arcSetAlgo(int * graph, int graph_len, struct edge * edges, int edges_len){
    randomGraphList(graph, graph_len);
    int arc_set_len = arcSetSize(edges, edges_len, graph, graph_len);
    struct edge arc_set_loop[arc_set_len];
    arcSet(edges, edges_len, graph, graph_len, arc_set_loop);
    if (true){
        outputSet(arc_set_loop, arc_set_len);
        for (int i = 0; i < graph_len; ++i) {
            printf("%d.", graph[i]);
        }
        printf("\n");
    }
    return arc_set_len;
}

static void readInput(int length, char **nargv, struct edge *pointer) {
    char *end;
    long int a;
    long int b;
    for (int i = 1; i < length + 1; i++) {

        a = strtol(nargv[i], &end, 10);
        if (*end != '-'){
            printf("[%s] Unvalid input, expected '-', but got '%c'.", nargv[0], *end);
            exit(EXIT_FAILURE);
        }
        b = strtol(end + 1, &end, 10);
        if ((a < 0) || (b < 0)) {
            printf("[%s] Unvalid input, parsed a negativ integer.", nargv[0]);
            exit(EXIT_FAILURE);
        }
        a = (int) a;
        b = (int) b;
        if ((a == INT_MAX) || (b == INT_MAX)){
            printf("[%s] Possible overflow, reached LONG_MAX: %d", nargv[0], errno);
            exit(EXIT_FAILURE);
        }
        struct edge edge1;
        edge1.a =  a;
        edge1.b =  b;
        pointer[i - 1] = edge1;
    }
}

static void randomGraphList(int * array, int len){
    int b;
    int a;
    int tmp;
    for (int i = 0; i < 20; ++i) {
        b = rand() % len;
        a = rand() % len;
        while (a == b){
            a = rand() % len;
        }
        tmp = *(array + a);
        *(array + a) = *(array + b);
        *(array + b) = tmp;
    }

}

static int graphLen(struct edge *s, int input_length){
    int max = 0;
    for (int i = 0; i < input_length; ++i) {
        if (s[i].a > max) {
            max = s[i].a;
        }
        if (s[i].b > max) {
            max = s[i].b;
        }
    }
    return max + 1;
}


static int getIndexInPermutation(int * array, int i, int len){
    for (int j = 0; j < len; ++j) {
        if (array[j] == i){
            return j;
        }
    }
    return -1;
}

static int arcSetSize(struct edge * edges, int edges_len, int * graph, int graph_len){
    int count = 0;
    int a, b;
    for (int i = 0; i < edges_len; ++i) {
        a = getIndexInPermutation(graph, edges[i].a, graph_len);
        b = getIndexInPermutation(graph, edges[i].b, graph_len);
        if (a > b){
            count++;
        }
    }
    return count;
}

static bool toBeSelected(struct edge s, int * graph, int graph_len){
    int a = getIndexInPermutation(graph, s.a, graph_len);
    int b = getIndexInPermutation(graph, s.b, graph_len);
    return a > b;
}

static void arcSet(struct edge * edges, int edges_len, int * graph, int graph_len, struct edge * arcSet){
    int setIndex = 0;
    for (int i = 0; i < edges_len; ++i) {
        if (toBeSelected(edges[i], graph, graph_len)){
            arcSet[setIndex] = edges[i];
            setIndex++;
        }
    }
}

void outputSet(struct edge * edges, int len){
    if (len == 0){
        printf(" The graph is acyclic!\n");
        return;
    }
    printf("Solution with %d edges:", len);
    for (int i = 0; i < len; ++i) {
        printf(" %d-%d",edges[i].a, edges[i].b);
    }
    printf("\n");

}

static void init(void){
    // create and/or open the shared memory object:
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == ENOENT){
        printf("[%s] No supervisor: first lauch .\\supervisor ", PROG_NAME);
        exit(EXIT_FAILURE);
    } else if (shmfd == -1 ){
        printf("[%s] Couldn't open shared memory!", PROG_NAME);
    }


    buff = mmap(NULL, sizeof(buff), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buff == MAP_FAILED){
        printf("[%s] Couldn't map memory!", PROG_NAME);
        exit(EXIT_FAILURE);
    }
    if (close(shmfd) == -1){
        printf("[%s] Couldn't close field descriptor!", PROG_NAME);
        exit(EXIT_FAILURE);
    }
    s_read = sem_open(SEM_USED, 0);
    if (s_read == SEM_FAILED) {
        printf("[%s] failed to open semaphore!", PROG_NAME);
        exit(EXIT_FAILURE);
    }
    s_write = sem_open(SEM_FREE, 0);
    if (s_write == SEM_FAILED) {
        printf("[%s] failed to open semaphore!", PROG_NAME);
        exit(EXIT_FAILURE);
    }
    s_mutex = sem_open(SEM_MUTEX, 0);
    if (s_mutex == SEM_FAILED) {
        printf("[%s] failed to open semaphore!", PROG_NAME);
        exit(EXIT_FAILURE);
    }
}

static void write_to_buffer(struct arc_set a){
    if (sem_wait(s_mutex) == -1){
        //error
    }
    if (buff.terminate){
        exit(EXIT_SUCCESS);
    }
    buff.array[write] = a;

}

