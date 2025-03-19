# Sieve, an effecient prime generator
This project implements a concurrent segmented sieve of erathosthenes in c++. It also implements a python interface for it.

## Building
The c++ implementation can directly be used by including the `Sieve.hpp` header file (c++17).

Python:
```bash
pip3 install pybind11
clang++ -O3 -Wall -shared -std=c++17 -fPIC $(python3-config --include) $(python3 -m pybind11 --includes) Sieve-python.cpp -o sieve$(python3-config --extension-suffix)
```
Any c++ compiler should work. Now, the module is contained within the `sieve` folder. You can import it as `import sieve`.