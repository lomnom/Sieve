#include "Timetest.hpp"
#include "Sieve.hpp"

using namespace std;

void sieve_test(num limit){
	list<num> result;
	cout << "Testing primes till " << limit << '\n';
	{
		TimeTest timer;
		result = parallel_sieve(limit);
	}

	cout << result.size() << " primes found\n";
	if (result.size() >= 100){
		cout << "Last 100 primes: ";
		auto it=result.rbegin();
		for (num i=0; i<100; i++){
			cout << *it << ' ';
			it++;
		}
	} else {
		cout << "Primes: " ;
		for (auto prime: result){
			cout << prime << ", ";
		}
	}
	cout << endl;
}

int main(){
	cout << "Enter the upper limit of primes to find (i.e. find all primes inside [1, limit])" << endl;
	num limit;
	cin >> limit;

	cout << "Finding primes..." << endl;
	sieve_test(limit);
	
	return 0;
}