#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

// discussion at the end

//struct Thread
struct Thread
{
    uint64_t lower_bound;
    uint64_t upper_bound;
};

//is_perfect
static int is_perfect(uint64_t num)
{
    uint64_t i, sum;
    if (num < 2)
    {
        return 0;
    }
    for (i = 2, sum = 1; i * i <= num; i++)
    {
        if (num % i == 0)
        {
            sum += (i * i == num) ? i : i + num / i;
        }
    }
    int res = (sum == num);
    return res;
}

//work
static void *work(void *arg)
{
    struct Thread *t = (struct Thread *)arg;
    for (uint64_t i = (*t).lower_bound; i <= (*t).upper_bound; i++)
    {
        if (is_perfect(i))
        {
            printf("%llu\n", i);
        }
    }
    // flushing stdout to check for stuck numbers
    fflush(stdout);
}

//create and join threads
void threading(int threads, int verbose, uint64_t upper_bound, uint64_t lower_bound)
{
    //allocating memory dynamically for the thread ids
    pthread_t *tids = (pthread_t *)malloc(sizeof(pthread_t) * threads);
    //checking pointer
    if (tids == NULL)
    {
        perror("error allocating memory for thread ids array");
    }
    //allocating memory for the array with structs
    struct Thread *threadArray = (struct Thread *)malloc(sizeof(struct Thread) * threads);
    //checking pointer
    if (threadArray == NULL)
    {
        perror("error allocating memory for thread structs array");
    }
    //calculating interval length
    uint64_t i = (upper_bound - lower_bound) / threads;
    //creating indexes keeping track of intervals
    uint64_t start = lower_bound;
    uint64_t end = lower_bound + i;
    int n = 1;
    while (n < threads)
    {
        if (verbose)
        {
            fprintf(stderr, "perfect: t%d searching [%llu , %llu]\n", n - 1, start, end);
        }
        //put struct attributes in a struct in the array
        threadArray[n - 1].lower_bound = start;
        threadArray[n - 1].upper_bound = end;
        // error handling pthread_create
        int rc = pthread_create(&tids[n - 1], NULL, work, &threadArray[n - 1]);
        if (rc)
        {
            perror("error creating thread");
        }
        //move indexes to next interval
        start = end + 1;
        end = start + i;
        n++;
    }
    //last or only interval
    if (verbose)
    {
        fprintf(stderr, "perfect: t%d searching [%llu , %llu]\n", n - 1, start, upper_bound);
    }
    //put struct attributes in a struct in the array
    threadArray[n - 1].lower_bound = start;
    threadArray[n - 1].upper_bound = upper_bound;
    // error handling pthread_create
    int rc = pthread_create(&tids[n - 1], NULL, work, &threadArray[n - 1]);
    if (rc)
    {
        perror("error creating thread");
    }
    // loop to join the threads back
    for (int i = 0; i < n; i++)
    {
        if (verbose)
        {
            fprintf(stderr, "perfect: t%d finishing\n", i);
        }
        int rc = pthread_join(tids[i], NULL);
        // error handling pthread_join
        if (rc)
        {
            perror("error joining thread");
        }
    }

    // free malloced memory
    free(threadArray);
    free(tids);
}

int main(int argc, char *argv[])
{
    //get options and option arguments
    int opt, upper_bound = 10000, lower_bound = 1, threads = 1, verbose = 0;
    while ((opt = getopt(argc, argv, "e:s:t:v")) != -1)
    {
        switch (opt)
        {
        case 'e':
            upper_bound = atoi(optarg);
            break;
        case 's':
            lower_bound = atoi(optarg);
            break;
        case 't':
            threads = atoi(optarg);
            break;
        case 'v':
            verbose = 1;
            break;
        default:
            break;
        }
    }
    // create, do something and then join threads
    threading(threads, verbose, upper_bound, lower_bound);
    return EXIT_SUCCESS;
}

// as the graph shows us, when we increase the number of threads 
// our program gets executed faster. This happens because threads run in parallel 
// so doing partial things at once is more efficient than doing the whole thing 
// sequentially ( or partial things concurrently ).
// We can see that system time increases a little bit, that is because the pthread_create
// function is a "system call" and increasing the number of threads increases 
// the number of those calls to kernel space. 
// Next, we can see that user space time is growing, that is because of 
// the loops when creating and then joining the threads, the more threads, the more looping.
// Finally, we can see that real time is decreasing, when the number of threads goes
// up. This is because parallelism is more efficient than concurrency.