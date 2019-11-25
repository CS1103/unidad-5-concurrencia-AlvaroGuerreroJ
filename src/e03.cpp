#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <thread>
#include <unordered_set>
#include <vector>

#include "prettyprint.hpp"

std::vector<unsigned> primes_til_n(unsigned n)
{
    std::list<unsigned> primes(n - 1);
    std::iota(primes.begin(), primes.end(), 2);

    size_t number_threads = std::thread::hardware_concurrency();

    auto cur_prime = primes.begin();
    while (std::next(cur_prime) != primes.end())
    {
        auto b = std::next(cur_prime);

        size_t remaining = std::distance(b, primes.end());
        size_t elements_per_thread = (size_t)
            std::ceil(double(remaining) / number_threads);

        std::vector<std::thread> threads;
        while (b != primes.end())
        {
            auto it = (remaining > elements_per_thread)
                      ? std::next(b, elements_per_thread)
                      : std::next(b, remaining);
            threads.emplace_back([&](auto b, auto e) {
                while (b != e)
                {
                    if (*b % *cur_prime == 0)
                    {
                        // Erasing in list only invalidate the iterator erased
                        b = primes.erase(b);
                    }
                    else
                    {
                        b++;
                    }
                }
            }, b, it);
            b = it;
            remaining -= elements_per_thread;
        }

        for (auto& t : threads)
        {
            t.join();
        }

        std::advance(cur_prime, 1);
    }

    return std::vector<unsigned>(primes.begin(), primes.end());
}

int main()
{
    std::cout << primes_til_n(100) << std::endl;
}
