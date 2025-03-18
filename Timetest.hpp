#include <chrono>
#include <iostream>

typedef long long int ll;

#define datetime std::chrono

/*
	The TimeTest class is a code timer which works like std::lock_guard.
	When it is constructed, it starts a timer. It stops the timer and prints the
	  time elapsed when it is destroyed (like when it goes out of scope).
	You can use it like:
		TimeTest tester("description to print");
		[code to time]

	Or to leave no description.
		TimeTest tester;
		[code to time]

	A useful tip to know is that you can surround a chunk in curly braces {} to 
	  make that chunk a seperate scope, to only time that chunk within a wider
	  context.
		[code...]
		{
			TimeTest tester;
			[code to time]
		}
		[code...]
*/

class TimeTest {
datetime::time_point<datetime::high_resolution_clock> start_time;
public:
	std::string text;

	// Default constructor, no desciption
	TimeTest(){
		start_time = datetime::high_resolution_clock::now();
	}

	// Constructor with description
	TimeTest(std::string _text): text(_text){
		start_time = datetime::high_resolution_clock::now();
	}

	// End the time when object is destroyed.
	~TimeTest(){
		datetime::time_point end_time = datetime::high_resolution_clock::now();
		ll microseconds = datetime::duration_cast<datetime::microseconds>(end_time - start_time).count();
		if (text.size() > 0){
			std::cout << text << " took " << microseconds/1000 << '.' << microseconds%1000 << "ms" << std::endl;
		} else {
			std::cout << "Took " << microseconds/1000 << '.' << microseconds%1000 << "ms" << std::endl;
		}
	}
};

#undef datetime