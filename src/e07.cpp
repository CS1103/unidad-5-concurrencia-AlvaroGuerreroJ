#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <random>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "prettyprint.hpp"

namespace
{
    template<class ForwardIter, class BinaryPred>
    void merge_ranges(ForwardIter b0, ForwardIter b1, ForwardIter e1,
                      BinaryPred cmp)
    {
        // This function is ugly. To avoid having to copy the range [b0, b1)
        // a std::deque<ForwardIter> vanished_from_r0 is created. Thanks to
        // that we reduce the storage cost (because ForwardIterator is
        // probably smaller than ForwardIterator::value_type at the extent of
        // some runtime performance.

        std::deque<ForwardIter> vanished_from_r0;
        auto e0 = b1;
        while (b0 != e0)
        {
            if (vanished_from_r0.empty())
            {
                if (cmp(*b1, *b0))
                {
                    std::iter_swap(b0, b1);
                    vanished_from_r0.push_back(b1);
                    b1++;
                }
            }
            else
            {
                if (cmp(*b1, *vanished_from_r0.front()))
                {
                    std::iter_swap(b0, b1);
                    vanished_from_r0.push_back(b1);
                    b1++;
                }
                else
                {
                    std::iter_swap(b0, vanished_from_r0.front());
                    vanished_from_r0.push_back(vanished_from_r0.front());
                    vanished_from_r0.pop_front();
                }
            }
            b0++;
        }

        while (b1 != e1 && !vanished_from_r0.empty())
        {
            // I suspect this could be logarithmic as vanished_from_r0 is
            // ordered in ascending order of dereference.
            auto rp = std::find(vanished_from_r0.begin(),
                                vanished_from_r0.end(), b0);

            if (cmp(*b1, *vanished_from_r0.front()))
            {
                std::iter_swap(b0, b1);
                *rp = b1;
                b1++;
            }
            else
            {
                std::iter_swap(b0, vanished_from_r0.front());
                *rp = vanished_from_r0.front();
                vanished_from_r0.pop_front();
            }
            b0++;
        }
        while (!vanished_from_r0.empty())
        {
            auto rp = std::find(vanished_from_r0.begin(),
                                vanished_from_r0.end(), b0);

            std::iter_swap(b0, vanished_from_r0.front());
            *rp = vanished_from_r0.front();
            vanished_from_r0.pop_front();
            b0++;
        }
        std::copy(b1, e1, b0);
    }
}

template<class ForwardIter, class BinaryPred>
void mergesort(ForwardIter b, ForwardIter e,
               typename ForwardIter::difference_type size,
               BinaryPred cmp)
{
    // If the range is empty or has one element, it's already sorted
    if (b == e || std::next(b, 1) == e)
    {
        return;
    }

    // Sort each halve of the range and merge them together
    // This is O(k) for random access iterators but O(n) for ForwardIter
    auto mid_point = std::next(b, size / 2);
    mergesort(b, mid_point, size / 2, cmp);
    mergesort(mid_point, e, size / 2 + size % 2, cmp);
    merge_ranges(b, mid_point, e, cmp);
}

template<class ForwardIter, class BinaryPred = std::less<>>
void mergesort(ForwardIter b, ForwardIter e, BinaryPred cmp = BinaryPred())
{
    auto size = std::distance(b, e);
    mergesort(b, e, size, cmp);
}

template<class ForwardIt>
void mergesort_concurrent(ForwardIt b, ForwardIt e)
{
    size_t number_threads = std::thread::hardware_concurrency();
    size_t total_size = std::distance(b, e);
    size_t elements_per_thread = (size_t)
        std::ceil(double(total_size) / number_threads);
    size_t remaining_elements = total_size;

    std::vector<std::pair<ForwardIt, ForwardIt>> ranges;
    while (b != e)
    {
        auto it = (remaining_elements > elements_per_thread)
                  ? std::next(b, elements_per_thread)
                  : std::next(b, remaining_elements);
        ranges.emplace_back(b, it);
        b = it;
        remaining_elements -= elements_per_thread;
    }

    std::vector<std::thread> threads;
    for (auto const& r : ranges)
    {
        threads.emplace_back([](ForwardIt b, ForwardIt e) {
            mergesort(b, e);
        }, r.first, r.second);
    }

    for (auto& t : threads)
    {
        t.join();
    }

    while (ranges.size() > 1)
    {
        std::vector<std::pair<ForwardIt, ForwardIt>> new_ranges;

        std::vector<std::thread> threads;
        for (size_t i = 0; i < ranges.size(); i += 2)
        {
            if (i + 1 < ranges.size())
            {
                new_ranges.emplace_back(ranges[i].first, ranges[i + 1].second);
                threads.emplace_back([](ForwardIt b0, ForwardIt e0,
                ForwardIt b1, ForwardIt e1) {
                    std::vector<typename ForwardIt::value_type> temp;
                    std::merge(b0, e0, b1, e1, std::back_inserter(temp));
                    std::copy(temp.begin(), temp.end(), b0);
                },
                ranges[i].first, ranges[i].second,
                ranges[i + 1].first, ranges[i + 1].second);
            }
            else
            {
                // XXX: This could be improved, the last range could be merged
                // with the previous one.
                new_ranges.emplace_back(ranges[i].first, ranges[i].second);
            }
        }

        for (auto& t : threads)
        {
            t.join();
        }

        ranges = new_ranges;
    }
}

int main()
{
    std::vector<unsigned> vi(100);
    std::generate(vi.begin(), vi.end(), []() {
        static std::random_device rd;
        return rd() % 100;
    });

    std::cout << vi << std::endl;
    mergesort_concurrent(vi.begin(), vi.end());
    std::cout << vi << std::endl;
}
