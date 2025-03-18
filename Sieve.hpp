/*
This implements a segmented Sieve of Eratosthenes with concurrency.
*/

#include <thread>
#include <atomic>
#include <vector>
#include <bitset>
typedef unsigned long long int num;

#define DEFAULT_THREADS 16 // Default number of extra threads to spawn to run the sieve
#define DEFAULT_CHUNK_SIZE (4*10e6) // Size of interval allocated to each thread at a time

/*
Lemma 1: If a number N is composite, it has at least one prime factor less than or equal to sqrt(N)
Proof: 
1. Suppose we have a number N, and it is composite. 
2. Thus, we have an array K of primes that when multiplied together result in N.
3. Assume that every prime factor is more than sqrt(n). Notice that since N is a 
     composite number, the list will contain at least two factors, both > sqrt(N).
4. Thus, the product of all factors in the list is more than N.
5. By contradiction, this proves lemma 1.
*/

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
	return;
}

std::vector<num> sieve_master(num upper_bound){
	return {1,2};
}

std::vector<num> parallel_sieve(num upper_bound){
	return {1,2};
}

// Naive sieves for cross-checking and performance comparison
std::vector<num> naive_sieve(num upper_bound){
	std::vector<num> primes; // Result array
	std::vector<bool> states(upper_bound, true);
	#define state(number) states[(number)-1] // Deal with zero-indexing

	num checking = 2;
	while (checking <= upper_bound){
		// At every iteration assume that all relevant primes have been tested against.
		if (state(checking) == true){
			primes.push_back(checking);

			// From lemma 1, we only need to check all primes less than or equal to sqrt(n)
			//   to determine if it is prime.
			// By starting at checking^2, if N is exactly checking^2, we do check N.
			//   i.e. if N = checking^2 -> checking = sqrt(n)
			// If any N is larger than checking^2, it will also be checked.
			//   i.e. N > checking^2 -> checking < sqrt(n)
			// Thus for every number, the requirements for lemma 1 are satisfied.
			for (num index = checking*checking; index <= upper_bound; index+=checking){
				state(index) = false;
			}
		}
		checking++;
	}

	return primes;
	#undef state
}
