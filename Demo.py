import sieve
import time

limit = None
while limit is None:
	limit = input("Enter an upper limit of primes to search for (i.e. find all primes in [1, limit]):\n")
	try:
		limit = int(limit)
	except ValueError:
		print("Invalid number!")

print("Finding primes...")

start = time.perf_counter()
primes = sieve.sieve(limit)
print(f"Took {time.perf_counter() - start}s to find primes. {len(primes)} primes found.")

if len(primes) > 100:
	print(f"Last 100 primes: {primes[-100:]}")
else:
	print(f"Primes found: {primes}")