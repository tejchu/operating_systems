#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

typedef struct prison
{
    int seed;
    int prisoners_n;
    int drawers_n;
    int games;
    pthread_mutex_t global_lock;
    pthread_mutex_t mutex_array[100];
} prison_t;

typedef struct prisoner
{
    int *drawers;
    int prisoner_number;
    int win;
} prisoner_t;

typedef struct environment
{
    prison_t *prison;
    prisoner_t prisoner;
} env_t;

// init prison struct
void init_prison(prison_t *prison)
{
    prison->drawers_n = 100;
    prison->prisoners_n = 100;
    prison->games = 100;
    prison->seed = 1;
    for (int i = 0; i < prison->drawers_n; i++)
    {
        pthread_mutex_init(&prison->mutex_array[i], NULL);
    }
    pthread_mutex_init(&prison->global_lock, NULL);
}

void init_prisoner(prisoner_t *prisoner, int n, int drawers[])
{
    prisoner->drawers = drawers;
    prisoner->prisoner_number = n + 1;
    prisoner->win = 0;
}

// helper functions with error handling
static void mutex_lock(pthread_mutex_t *mutex)
{
    int rc = pthread_mutex_lock(mutex);
    if (rc)
    {
        fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }
}
static void mutex_unlock(pthread_mutex_t *mutex)
{
    int rc = pthread_mutex_unlock(mutex);
    if (rc)
    {
        fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }
}

// random array generator
void generate_random_array(int rand_arr[], prison_t *prison)
{
    int max = prison->prisoners_n, n, i, initial_arr[prison->prisoners_n];
    for (i = 0; i < max; i++)
    {
        initial_arr[i] = i + 1;
    }
    i = 0;
    mutex_lock(&prison->global_lock);
    srand(prison->seed++);
    mutex_unlock(&prison->global_lock);
    while (max > 0)
    {
        n = rand() % max;
        rand_arr[i] = initial_arr[n];
        initial_arr[n] = initial_arr[max - 1];
        i++;
        max--;
    }
}

// strategies
// random drawers with global lock
static void *random_global(void *arg)
{
    env_t *p = (env_t *)arg;
    int drawer_array[p->prison->drawers_n];
    generate_random_array(drawer_array, p->prison);
    mutex_lock(&p->prison->global_lock);
    for (int i = 0; i < 50; i++)
    {
        int open = drawer_array[i] - 1;
        // open random drawer
        if (p->prisoner.drawers[open] == p->prisoner.prisoner_number)
        {
            p->prisoner.win = 1;
            break;
        }
    }
    mutex_unlock(&p->prison->global_lock);
    return EXIT_SUCCESS;
}

// random drawer with local locks
static void *random_drawer(void *arg)
{
    env_t *p = (env_t *)arg;
    int open = 0;
    int drawer_array[p->prison->drawers_n];
    generate_random_array(drawer_array, p->prison);
    for (int i = 0; i < 50; i++)
    {
        open = drawer_array[i] - 1;
        // open random
        mutex_lock(&p->prison->mutex_array[open]);
        if (p->prisoner.drawers[open] == p->prisoner.prisoner_number)
        {
            p->prisoner.win = 1;
            mutex_unlock(&p->prison->mutex_array[open]);
            break;
        }
        else
        {
            mutex_unlock(&p->prison->mutex_array[open]);
        }
    }
    return EXIT_SUCCESS;
}

// opening a drawer with strategy using individual locks
static void *strategy_drawer(void *arg)
{
    env_t *p = (env_t *)arg;
    int open = p->prisoner.prisoner_number - 1;
    for (int i = 0; i < 50; i++)
    {

        // open drawer with prisoners_n number
        mutex_lock(&p->prison->mutex_array[open]);

        if (p->prisoner.drawers[open] == p->prisoner.prisoner_number)
        {
            mutex_unlock(&p->prison->mutex_array[open]);
            p->prisoner.win = 1;
            break;
        }
        else
        {
            mutex_unlock(&p->prison->mutex_array[open]);
            open = p->prisoner.drawers[open] - 1;
        }
    }
    return EXIT_SUCCESS;
}

// opening a sequence of drawers with strategy using global lock
static void *strategy_global(void *arg)
{
    env_t *p = (env_t *)arg;
    int open = p->prisoner.prisoner_number - 1;
    mutex_lock(&p->prison->global_lock);
    for (int i = 0; i < 50; i++)
    {
        // open drawer with prisoners_n number
        if (p->prisoner.drawers[open] == p->prisoner.prisoner_number)
        {
            p->prisoner.win = 1;
            break;
        }
        else
        {
            open = p->prisoner.drawers[open] - 1;
        }
    }
    mutex_unlock(&p->prison->global_lock);
    return EXIT_SUCCESS;
}

// helper functions

static void run_threads(void *(*proc)(void *), int *wins, prison_t *prison)
{
    int i = 0, sum = 0;
    // generating drawers with random numbers
    int drawers[prison->drawers_n];
    generate_random_array(drawers, prison);
    // allocating memory dynamically for the thread ids
    pthread_t *thread_ids = (pthread_t *)malloc(sizeof(pthread_t) * prison->prisoners_n + 1);
    // allocating memory for the array with structs Prisoner
    env_t *prisoners_array = (env_t *)malloc(sizeof(env_t) * prison->prisoners_n + 1);
    // checking pointers
    if (thread_ids == NULL)
    {
        perror("error allocating memory for thread ids array");
    }
    if (prisoners_array == NULL)
    {
        perror("error allocating memory for thread structs array");
    }

    while (i < prison->prisoners_n)
    {
        prisoner_t prisoner;
        init_prisoner(&prisoner, i, drawers);
        prisoners_array[i].prison = prison;
        prisoners_array[i].prisoner = prisoner;
        // error handling pthread_create
        int rc = pthread_create(&thread_ids[i], NULL, proc, &prisoners_array[i]);
        if (rc)
        {
            fprintf(stderr, "pthread_create(): %s\n", strerror(rc));
            exit(EXIT_FAILURE);
        }
        // move index
        i++;
    }
    // loop to join the threads back
    for (int i = 0; i < prison->prisoners_n; i++)
    {
        if (prisoners_array[i].prisoner.win == 1)
        {
            sum++;
            if (sum == 100)
            {
                *wins += 1;
            }
        }
        int rc = pthread_join(thread_ids[i], NULL);
        // error handling pthread_join
        if (rc)
        {
            fprintf(stderr, "pthread_join(): %s\n", strerror(rc));
            exit(EXIT_FAILURE);
        }
    }

    // free malloced memory
    free(prisoners_array);
    free(thread_ids);
}

static double timeit(void *(*proc)(void *), int *wins, prison_t *prison)
{
    clock_t t1, t2;
    t1 = clock();
    run_threads(proc, wins, prison);
    t2 = clock();
    return ((double)t2 - (double)t1) / CLOCKS_PER_SEC * 1000;
}

static void
run_simulation(int *wins, prison_t *prison, void *(*proc)(void *), char *method_name)
{
    double time = 0;
    for (int i = 0; i < prison->games; i++)
    {
        time += timeit(proc, wins, prison);
    }
    printf("method %s\t%d/%d wins = %.2f %%\t%.3f ms\n", method_name, *wins, prison->games, ((double)*wins / prison->games) * 100, time);
}

int main(int argc, char *argv[])
{
    int opt, wins_s_d = 0, wins_s_g = 0, wins_r_d = 0, wins_r_g = 0;
    prison_t prison;
    init_prison(&prison);
    while ((opt = getopt(argc, argv, "n:s:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            prison.games = atoi(optarg);
            break;
        case 's':
            prison.seed = atoi(optarg);
            break;
        default:
            break;
        }
    }

    // creating mutex for every drawer
    run_simulation(&wins_r_d, &prison, random_drawer, "random_drawer");
    run_simulation(&wins_r_g, &prison, random_global, "random_global");
    run_simulation(&wins_s_d, &prison, strategy_drawer, "strategy_drawer");
    run_simulation(&wins_s_g, &prison, strategy_global, "strategy_global");

    fflush(stdout);
    return EXIT_SUCCESS;
}


