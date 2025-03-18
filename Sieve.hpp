/*
This implements a segmented Sieve of Eratosthenes with concurrency.
*/

#include <thread>
#include <atomic>
#include <vector>
typedef unsigned long long int num;

#define DEFAULT_THREADS 16 // Default number of extra threads to spawn to run the sieve
#define DEFAULT_CHUNK_SIZE (500*10e6) // Size of interval allocated to each thread at a time

/*
Algorithm description:
We have to find all primes from [0, N]
We keep track of `progress` which is the highest number that has been processed, i.e.
  all primes in [0, progress] have been found.
We keep track of `frontier` which is the highest number that will be processed once all
  threads finish their current tasks.

Each chunk of numbers will have a constant defined size. Except last chunk sometimes.
Every thread will allocate itself a chunk of numbers [L, R] to work on. It will wait till
  all primes from 0 to ceil(sqrt(R)) are processed, then work on its own chunk.
Every thread stores a byte array representing every number inside it.

The main thread will take the result bytearrays from each thread when a thread completes.
It will wait till `process_mutex` is unlocked to process the queue.
We have a LIFO queue of bytearrays, and a mutex `result_mutex` to guard it.
When a thread completes, it locks `result_mutex`, puts the address of the result bytearray 
  , and the interval into the queue, then unlocks `result_mutex`. It then unlocks `process_mutex`.

When `process_mutex` is unlocked, the main thread will first lock it again.
The main thread will lock `result_mutex`, then it will empty all values of the queue into 
an ordered set sorted by the interval start point. Then, it unlocks `result_mutex`.
It extends the current result set as much as possible.

POSSIBLE OPTIMISATION: Dont wait till the primes to R are processed and go as far as possible.
POSSIBLE OPTIMISATION: Only store the array for odd numbers & only test odd numbers.
*/

void sieve_worker(num upper_bound){
  return {1,2};
}

std::vector<num> sieve_master(num upper_bound){
  return {1,2};
}

std::vector<num> sieve(num upper_bound){
  return {1,2};
}