# Sieve, an effecient prime generator
This project implements a concurrent segmented sieve of erathosthenes in c++. It also implements a python interface for it.

## Building
The c++ implementation can directly be used by including the `Sieve.hpp` header file (c++17).

Python:
```bash
pip3 install pybind11
# Ensure python is installed with developer headings
# Long ahh command from https://pybind11.readthedocs.io/en/stable/basics.html
g++ -O3 -Wall -shared -std=c++17 -fPIC $(python3-config --includes) $(python3 -m pybind11 --includes) Sieve-python.cpp -o sieve$(python3-config --extension-suffix)
```
Any c++ compiler should work g++. You can import it as `import sieve` from python after compilation.

## Demos
The c++ demo is relatively easy to build.
```bash
clang++ -std="c++17" -Wall Demo.cpp
./a.out
```
All c++ compilers should work in place of clang.

The python demo requires building the library with steps in #building. Then run Demo.py.
```bash
python3 Demo.py
```