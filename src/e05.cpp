#include <cmath>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <vector>

#include "prettyprint.hpp"

template<class T>
void remove_element(std::vector<T>& v, T element_to_remove)
{
    size_t number_threads = std::thread::hardware_concurrency();
    size_t elements_per_thread = (size_t)
        std::ceil(double(v.size()) / number_threads);

    std::unordered_set<size_t> indexes_to_remove;
    std::vector<std::thread> threads;

    size_t b = 0;
    size_t e = v.size();

    while (b != e)
    {
        auto it = (b + elements_per_thread > e)
                  ? e
                  : b + elements_per_thread;
        threads.emplace_back([&](size_t b, size_t e) {
            while (b != e)
            {
                if (v[b] == element_to_remove)
                {
                    indexes_to_remove.insert(b);
                }
                b++;
            }
        }, b, it);
        b = it;
    }

    for (auto& t : threads)
    {
        t.join();
    }

    size_t cur_insert = 0;
    size_t cur_pos = 0;
    while (cur_pos != v.size())
    {
        if (indexes_to_remove.count(cur_pos) == 0)
        {
            v[cur_insert] = v[cur_pos];
            cur_insert++;
        }
        cur_pos++;
    }
    v.erase(std::next(v.begin(), cur_insert), v.end());
}

int main()
{
    std::vector<int> vi = {99, 1, 99, 2, 3, 99, 4, 99, 99, 99, 5, 99, 6, 99, 99};

    std::cout << vi << std::endl;
    remove_element(vi, 99);
    std::cout << vi << std::endl;
}
