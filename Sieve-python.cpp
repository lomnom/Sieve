#include <pybind11/pybind11.h>
#include "Sieve.hpp"

std::list<num> sieve_interface(num upper_bound){
    if (upper_bound > 4250000000){
        throw std::invalid_argument("upper_bound for the sieve must not be bigger than 4250000000!");
    }
    return parallel_sieve(upper_bound);
}

PYBIND11_MODULE(conc_sieve, module)
{
    module.doc() = "Implements a parallel sieve of eratosthenes in sieve(upper_bound)."
                   "Finds all primes in [1, upper_bound]."; // optional module docstring
    module.def("sieve", &sieve_interface);
}