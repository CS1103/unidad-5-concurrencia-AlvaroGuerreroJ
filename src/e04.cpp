#include <algorithm>
#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "prettyprint.hpp"

unsigned fib_n(unsigned n)
{
    static double phi = (std::sqrt(5) + 1) / 2;
    return (std::pow(phi, n) - pow(-1 / phi, n)) / pow(5, .5);
}

std::vector<unsigned> fibs_til_n(unsigned n)
{
    size_t number_threads = std::thread::hardware_concurrency();
    size_t elements_per_thread = (size_t)
        std::ceil(double(n) / number_threads);

    size_t remaining_elements = n;
    unsigned b = 0;
    std::vector<std::thread> threads;

    std::mutex ret_mutex;
    std::vector<unsigned> ret;

    while (b != n)
    {
        auto it = (remaining_elements > elements_per_thread)
                  ? b + elements_per_thread
                  : n;
        threads.emplace_back([&](auto b, auto e) {
            std::vector<unsigned> temp;
            while (b != e)
            {
                temp.emplace_back(fib_n(b));
                b++;
            }
            std::lock_guard<std::mutex> lm(ret_mutex);
            auto it = std::lower_bound(ret.begin(), ret.end(), temp.front());
            ret.insert(it, temp.begin(), temp.end());
        },
        b, it);
        b = it;
        remaining_elements -= elements_per_thread;
    }

    for (auto& t : threads)
    {
        t.join();
    }

    return ret;
}

int main()
{
    std::cout << fibs_til_n(40) << std::endl;
}
