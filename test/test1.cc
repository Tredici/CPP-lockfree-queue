#include "../modules/CPP-test-unit/tester.hh"
#include "../fixed_size_lockfree_queue.hh"

#include <thread>
#include <vector>
#include <iostream>
#include <string>

tester t1([]{
    // how many test to generate
    constexpr std::size_t N = 1000;
    // how many pair of workerd
    auto t_count = 1000;
    std::vector<std::thread> workers;
    lockfree_queue::fixed_size_lockfree_queue<int> q(N);

    // spawn workers
    for (int i{}; i!=t_count; ++i) {
        // add producer
        workers.emplace_back([&q](){
            for (int i{}; i!=N; ++i) {
                std::unique_ptr<int> up(new int); 
                *up = i;
                while (!q.offer(up));
            }
            //std::cout << "Producer exited!\n";
        });
        // add consumer
        workers.emplace_back([&q](){
            for (int i{}; i!=N; ++i) {
                std::unique_ptr<int> up;
                while (!q.poll(up));
            }
            //std::cout << "Consumer exited!\n";
        });
    }

    // join thread
    for (auto& t : workers) {
        t.join();
        //std::cout << "Thread terminated\n";
    }
});


tester t2([]{
    // how many test to generate
    constexpr std::size_t N = 1000;
    // how many pair of workerd
    auto t_count = 1000;
    std::vector<std::thread> workers;
    lockfree_queue::fixed_size_lockfree_queue<int> q(N);
    std::vector<std::atomic_int> counter(N);

    // spawn workers
    for (int i{}; i!=t_count; ++i) {
        // add producer
        workers.emplace_back([&q](){
            for (int i{}; i!=N; ++i) {
                std::unique_ptr<int> up(new int); 
                *up = i;
                while (!q.offer(up));
            }
            //std::cout << "Producer exited!\n";
        });
        // add consumer
        workers.emplace_back([&q, &counter](){
            for (int i{}; i!=N; ++i) {
                std::unique_ptr<int> up;
                while (!q.poll(up));
                ++counter.at(*up);
            }
            //std::cout << "Consumer exited!\n";
        });
    }

    // join thread
    for (auto& t : workers) {
        t.join();
        //std::cout << "Thread terminated\n";
    }
    return;
    for (auto& c : counter)
    {
        if (c.load() != t_count) {
            using namespace std::literals;
            throw std::runtime_error("Error counter"s);
        }
    }
});