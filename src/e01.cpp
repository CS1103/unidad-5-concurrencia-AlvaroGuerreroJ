#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

std::random_device rd;

void exercise1()
{
    const size_t n = 1200;
    auto nt = std::thread::hardware_concurrency();
    auto range = n / nt;

    std::vector<int> v(n);

    auto filler = [&v](size_t start, size_t stop) {
        for (unsigned i = start; i < stop; i++)
        {
            v[i] = rd() % 10000;
        }
    };

    std::vector<std::thread> vt(nt);

    for (unsigned i = 0; i < nt; i++)
    {
        vt[i] = std::thread(filler, range * i, range * (i + 1));
    }

    for (auto& t : vt)
    {
        t.join();
    }

    std::vector<std::promise<int>> vpmr(nt);
    std::vector<std::future<int>> vftr;

    for (auto& pmr : vpmr)
    {
        vftr.emplace_back(pmr.get_future());
    }

    auto summarizer = [&v](std::promise<int>& prm, size_t start, size_t stop) {
        unsigned total = 0;
        for (unsigned i = start; i < stop; i++)
        {
            total += v[i];
        }
        prm.set_value(total);
    };

    std::vector<std::thread> vt2(nt);
    for (unsigned i = 0; i < nt; i++)
    {
        vt2[i] = std::thread(summarizer, std::ref(vpmr[i]), range * i,
                             range * (i + 1));
    }

    for (auto& t : vt2)
    {
        t.join();
    }

    auto total = std::accumulate(vftr.begin(), vftr.end(), 0,
                                 [](int rsf, std::future<int>& ftr) {
                                     return rsf + ftr.get();
                                 });

    std::cout << total << std::endl;
    std::cout << std::accumulate(v.begin(), v.end(), 0) << std::endl;
}

int main()
{
    std::cout << std::thread::hardware_concurrency() << std::endl;
    exercise1();
}
