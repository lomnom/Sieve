/*
This implements a segmented Sieve of Eratosthenes with concurrency.
Due to integer overflows, this algorithm can currently only generate up to a limit of sqrt(2^64 - 1).
Just to be safe, avoid setting limits above 4.25 billion = 4250000000.
*/

#include <thread>
#include <atomic>
#include <vector>
// #include <bitset>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <tuple>
#include <limits>
typedef unsigned long long int num;

#define DEFAULT_THREADS 16 // Default number of extra threads to spawn to run the sieve
// Size of interval allocated to each thread at a time
// #define DEFAULT_CHUNK_SIZE (1*1e6) // 1 million
#define DEFAULT_CHUNK_SIZE 100000 // Somehow the fastest

/*
Lemma 1: If a number N is composite, it has at least one prime factor <= sqrt(N)
Proof: 
1. Suppose we have a number N, and it is composite. 
2. Thus, we have an array K of primes that when multiplied together result in N.
3. Assume that every prime factor is more than sqrt(n). Notice that since N is a 
     composite number, the list will contain at least two factors, both > sqrt(N).
4. Thus, the product of all factors in the list is more than N.
5. By contradiction, this proves lemma 1.

Lemma 2: For every prime P, we only need to check it against all n where N >= P^2
           to determine if N is prime.
Proof: 
1. From lemma 1, for every number N, only all primes <= sqrt(N) have to be checked against N.
2. We consider a prime P. Checking a number N against P is wasteful if P > sqrt(N). All 
     primes <= sqrt(n) would already be sufficient.
3. Thus, for a prime P, we should only check a number N where P <= sqrt(N) -> N >= P^2.
*/

// Sieve worker accepts small chunks of the whole interval to process at a time.
void sieve_worker( // Basically inheriting local vars from `parallel_sieve`
	const num id,
	const num upper_bound,
	const num chunk,
	std::list<num>& primes, 
	std::atomic<num>& progress, 
	num& frontier, 
	std::mutex& task_taking_mutex,
	std::stack< std::tuple<num, num, std::vector<bool>* > >& result_stack, 
	std::mutex& result_mutex, 
	std::mutex& process_mutex, 
	std::vector<std::mutex>& new_task_signals,
	std::list<num>::iterator& end_iterator,
	std::mutex& iterator_mutex
){
	while (1){
		// Wait till a new task has appeared
		new_task_signals[id].lock();

		num start, end;
		// Try to take task
		{
			std::lock_guard<std::mutex> lock(task_taking_mutex);
			if (frontier == upper_bound){
				// std::cout << "All done! Thread #" << id << " exiting" << std::endl;
				return; // No more tasks to take.
			}

			// Else, allocate a task.
			start = frontier + 1;
			end = (start + chunk - 1);
			if (end > upper_bound){
				end = upper_bound;
			}

			// Check if task is doable with current primes calculated.
			// By lemma #1, we need primes within [1, sqrt(end)] to have been calculated.
			// i.e., progress >= sqrt(end) -> progress^2 > end
			if (!( progress * progress > end )){ // NOTE: OVERFLOW WAITING TO HAPPEN!
				// std::cout << "Not enough primes calculated. " << "progress=" << progress << " but end=" << end << std::endl;
				continue; // We go back to waiting for more to process, as the latest task cannot be done yet.
			}

			frontier = end;
		}

		// std::cout << "Thread #" << id << " allocated [" << start << ", " << end << "]" << std::endl;

		iterator_mutex.lock();
		std::list<num>::iterator limit = end_iterator;
		iterator_mutex.unlock();

		// Now, our task is to find all primes in [start, end]
		std::vector<bool>* is_prime = new std::vector<bool>(end - start + 1, true);
		#define state(number) (*is_prime)[number - start] // Deal with starting at start.

		// We only need to check against all primes where prime <= sqrt(end)
		// From lemma 2, for every prime P we only need to start checking from N=P^2.
		auto it = primes.begin();
		while (1){ 
			num prime = *it;

			// Figure out where to start checking from.
			num checking = prime * prime; // NOTE: OVERFLOW WAITING TO HAPPEN
			if (checking > end){
				break; // All subsequent primes are not relevant too.
			} else if (start <= checking && checking <= end){
				// This checking position is valid
			} else {
				// The checking position falls short.
				// We need to find the first value divisible by prime which is in [start, end]

				// Rounded down to nearest chunk. Either below start, or at start.
				checking = (start/prime)*prime;
				if (checking < start) { // Got rounded to chunk before
					checking += prime;
				}
				// Now it is at start, or the first chunk which starts after start.
				// i.e. It is the first chunk which does not fall below start.

				if (checking > end){ // No multiple exists within the range.
					if (it != limit){
						it++;
					} else {
						break;
					}
					continue;
				}
				// If it passes this point, it is the first chunk which falls within [start, end]
			}

			// Run the sieve for the prime. The actual slow part.
			for (;checking <= end; checking += prime){
				state(checking) = false;
			}

			if (it != limit){
				it++;
			} else {
				break;
			}
		}

		// Now, we need to submit the results to master thread.
		result_mutex.lock();
		result_stack.push({start, end, is_prime});
		process_mutex.unlock(); // Open floodgates for process_mutex to process.
		result_mutex.unlock();
	}
	#undef state
}

// Sieve master will receive results from threads and merge them into the main list.
// It terminates when all primes have been found and merged.
void sieve_master( // Basically inheriting local vars from `parallel_sieve`
	const num upper_bound,
	const num threads,
	std::list<num>& primes, 
	std::atomic<num>& progress, 
	std::stack< std::tuple<num, num, std::vector<bool>* > >& result_stack, 
	std::mutex& result_mutex, 
	std::mutex& process_mutex, 
	std::vector<std::mutex>& new_task_signals,
	std::list<num>::iterator& end_iterator,
	std::mutex& iterator_mutex
){
	// Set of chunks waiting for previous chunks to do a continuous merge.
	std::map< std::pair<num, num>, std::vector<bool>* > pending_merge; 
	while (progress < upper_bound){
		process_mutex.lock(); // process_mutex is like a floodgate to allow the results to be flushed.

		result_mutex.lock();
		while (!result_stack.empty()){
			std::tuple<num, num, std::vector<bool>* > result = result_stack.top();
			result_stack.pop();
			pending_merge[{std::get<0>(result), std::get<1>(result)}] = std::get<2>(result);
		}
		result_mutex.unlock();

		// Unfortunately we still have to lock the primes array while merging.
		// While the frontmost chunk in queue is continuous if merged
		while (pending_merge.begin()->first.first == progress+1){ 
			auto it = pending_merge.begin(); // it->{{left, right}, pointer}
			num left = it->first.first;
			num right = it->first.second;
			std::vector<bool>* pointer = it->second;
			it = pending_merge.erase(it);

			// Now we merge that chunk in.
			for (num index = 0; index < (right - left + 1); index++){
				num number = left + index;
				if ((*pointer)[index] == true){
					primes.push_back(number);
				}
			}
			delete pointer;

			{
				std::lock_guard<std::mutex> lock(iterator_mutex);
				// std::cout << "\rProgressed to " << progress;
				progress = right;
				end_iterator = std::prev(primes.end(), 1);
			}
		}

		// Open the floodgates for every thread to try to find a new task as new tasks may have been created.
		for (num thread = 0; thread<threads; thread++){
			new_task_signals[thread].unlock();
		}
	}
}

// Naive sieves for cross-checking and performance comparison
// Container just has to support push_back
// eg. naive_sieve< vector<num> >(100) 
template <typename Container>
Container naive_sieve(num upper_bound){
	Container primes; // Result array
	std::vector<bool> states(upper_bound, true);
	#define state(number) states[(number)-1] // Deal with zero-indexing

	num checking = 2;
	while (checking <= upper_bound){
		// At every iteration assume that all relevant primes have been tested against.
		if (state(checking) == true){
			primes.push_back(checking);

			// Lemma 2.
			for (num index = checking*checking; index <= upper_bound; index+=checking){ // NOTE: OVERFLOW WAITING TO HAPPEN
				state(index) = false;
			}
		}
		checking++;
	}

	return primes;
	#undef state
}

std::list<num> parallel_sieve(num upper_bound, num threads = DEFAULT_THREADS, num chunk = DEFAULT_CHUNK_SIZE){
	// We need to first generate enough primes such that starting a new chunk, 
	//   all values would only depend on values already calculated.
	// Let N be the upper limit for primes needed (so we have all primes in [1, N]). N satisfies
	// ->  N >= floor(sqrt(N + chunk))
	// As for a chunk which ends at N+chunk, we need all primes <= sqrt(N+chunk)
	// We use N = floor(2 * sqrt(chunk)) to satisfy this.
	// List is used so multiple threads can read while appends happen.
	std::atomic<num> progress = (num)( floor(2 * sqrt(chunk)) ); // Always taken that all primes in [1, progress] are found.

	if (progress >= upper_bound){
		// std::cout << "Too small. pre=" << progress << " Just using normal sieve." << std::endl;
		return naive_sieve< std::list<num> >(progress);
	}

	std::list<num> primes = naive_sieve< std::list<num> >(progress); 
	
	std::mutex task_taking_mutex; // To be locked while accepting a task (reading/writing frontier)
	num frontier = progress; // The largest number which is not pending to be solved by any thread.
	// std::cout << "Initial=" << progress << std::endl;

	// Guaranteed to point to the biggest prime less than `progress`
	std::list<num>::iterator end_iterator = std::prev(primes.end(), 1); 
	std::mutex iterator_mutex; // To be locked before altering progress and altering/accessing end_iterator

	std::mutex result_mutex; // To be locked when result_stack is modified/read.
	std::stack< std::tuple<num, num, std::vector<bool>* > > result_stack; // Stores each result in a {L, R, &bitset}
	std::mutex process_mutex; // To be unlocked to signal to main thread to start processing new result.
	process_mutex.lock();

	// Unlocked by main thread when progress increases, to signal to threads that new tasks exist.
	std::vector<std::mutex> new_task_signals(threads); // Initialised unlocked.

	std::vector<std::thread> workers;
	for (num thread = 0; thread<threads; thread++){
		workers.emplace_back(
			sieve_worker, 
			thread, upper_bound, chunk,
			std::ref(primes), std::ref(progress), 
			std::ref(frontier), std::ref(task_taking_mutex), 
			std::ref(result_stack), std::ref(result_mutex), std::ref(process_mutex),
			std::ref(new_task_signals), 
			std::ref(end_iterator), std::ref(iterator_mutex)
		);
	}

	sieve_master(
		upper_bound, threads, 
		primes, progress, 
		result_stack, result_mutex, 
		process_mutex, new_task_signals, 
		end_iterator, iterator_mutex
	);

	for (auto& thread: workers) {
	    thread.join();
	}

	return primes;
}
