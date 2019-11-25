#include <cmath>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <thread>
#include <vector>

template<class ForwardIt, class T>
void increment(ForwardIt b, ForwardIt e, T inc_e)
{
    size_t number_threads = std::thread::hardware_concurrency();
    size_t total_size = std::distance(b, e);
    size_t elements_per_thread = (size_t)
        std::ceil(double(total_size) / number_threads);
    size_t remaining_elements = total_size;

    std::vector<std::thread> threads;
    while (b != e)
    {
        auto it = (remaining_elements > elements_per_thread)
                  ? std::next(b, elements_per_thread)
                  : std::next(b, remaining_elements);
        threads.emplace_back([&](auto b, auto e) {
            while (b != e)
            {
                *b += inc_e;
                b++;
            }
        }, b, it);
        b = it;
        remaining_elements -= elements_per_thread;
    }

    for (auto& t : threads)
    {
        t.join();
    }
}

int main()
{
    std::list<int> li(1000);
    std::iota(li.begin(), li.end(), 1);

    for (auto e : li)
    {
        std::cout << e << " ";
    }
    std::cout << std::endl;

    increment(li.begin(), li.end(), 1000);

    for (auto e : li)
    {
        std::cout << e << " ";
    }
    std::cout << std::endl;
}
