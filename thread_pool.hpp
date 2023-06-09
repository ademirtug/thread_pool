#pragma once


#include <type_traits>
#include <vector>
#include <future>
#include <queue>

class thread_pool {
    void loop_func() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock, [this] {
                    return bailout_ || !tasks_.empty();
                    });

                if (bailout_)
                    return;

                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }
    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> threads_;
    std::condition_variable cv_;
    std::mutex queue_mutex_;
    bool bailout_{ false };
public:
    thread_pool(size_t max = 10) {
        for (size_t i = 0; i <= max; i++) {
            threads_.push_back(std::thread(&thread_pool::loop_func, this));
        }
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> decltype(auto)
    {
        auto task = std::make_shared<std::packaged_task<typename std::invoke_result<F, Args...>
            ::type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<decltype(f(std::forward<Args>(args)...))> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace([task]() {
                (*task)();
            });
        }
        
        cv_.notify_one();
        return result;
    }

    ~thread_pool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            bailout_ = true;
        }
        cv_.notify_all();
        for (std::thread& th : threads_)
            th.join();
    }
};
