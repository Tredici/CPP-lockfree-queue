
#ifndef FIXED_SIZE_LOCKFREE_QUEUE
#define FIXED_SIZE_LOCKFREE_QUEUE

#include <memory>
#include <atomic>
#include <stdexcept>

namespace lockfree_queue
{
    template <typename T>
    class fixed_size_lockfree_queue
    {
    public:
        using value_type = T;
        using ptr_type = value_type*;
    private:
        std::size_t N;
        std::atomic_size_t sz{};
        std::unique_ptr<std::atomic<value_type*>[]> data;
        std::atomic_size_t insert_off{}, extract_off{};
    public:
        fixed_size_lockfree_queue(std::size_t N)
        : N{static_cast<decltype(this->N)>(N)}, data{new std::atomic<value_type*>[N]}
        {
            if (N <= 0) {
                throw std::out_of_range("N out of range");
            }
            for (std::remove_const_t<decltype(N)> i{}; i!=N; ++i) {
                data[i] = nullptr;
            }
        }

        ~fixed_size_lockfree_queue() {
            for (std::remove_const_t<decltype(N)> i{}; i!=N; ++i) {
                if (data[i] != nullptr) {
                    delete data[i];
                    data[i] = nullptr;
                }
            }
        }

        // prevent copy
        fixed_size_lockfree_queue(const fixed_size_lockfree_queue&) = delete;
        fixed_size_lockfree_queue& operator=(const fixed_size_lockfree_queue&) = delete;
        // allow moving
        fixed_size_lockfree_queue(fixed_size_lockfree_queue&&) = default;
        fixed_size_lockfree_queue& operator=(fixed_size_lockfree_queue&&) = default;

        // try to insert a new item, fails if there is no space
        // return true on success and false on failure
        bool offer(std::unique_ptr<T>& val) {
            if (!val) {
                throw std::invalid_argument("cannot insert nullptr");
            }
            bool success = false;
            // check for space available
            typename decltype(sz)::value_type size = sz;
            while (size < N && !sz.compare_exchange_weak(size, size+1));
            if (N > size) {
                // space available!
                // get insertion position!
                const auto pos = insert_off++ % N;
                // obtain value to use
                ptr_type ptr = val.release();
                ptr_type expected = nullptr;
                // ensure previous element has been taken
                auto& cel = data[pos];
                while (!cel.compare_exchange_weak(expected, ptr)) {
                    expected = nullptr;
                }
                success = true;
            }
            return success;
        }

        bool poll(std::unique_ptr<T>& val) {
            if (val.get() != nullptr) {
                throw std::invalid_argument("input val must be empty");
            }
            bool success = false;
            typename decltype(sz)::value_type size = sz;
            while (size != 0 && !sz.compare_exchange_weak(size, size-1));
            if (size > 0) {
                // non empty!
                // get extraction position!
                const auto pos = extract_off++ % N;
                // reference to the cell holding the value
                auto& cel = data[pos];
                ptr_type expected = nullptr;
                do {
                    expected = cel.exchange(nullptr);
                } while (expected == nullptr);
                val = std::unique_ptr<value_type>(expected);
                success = true;
            }
            return success;
        }

        // estimate the number of items inside the queue,
        // alwais between 0 and N-1
        std::size_t size() const {
            return sz;
        }

        bool empty() const {
            return size() == 0;
        }

        // Maximum number of elements that can be hold
        // by the queue
        std::size_t capacity() const {
            return N;
        }
    };
}

#endif
