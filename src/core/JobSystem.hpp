#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace veys {

class JobSystem {
public:
    JobSystem();
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    template <typename Fn, typename... Args>
    auto enqueue(Fn&& fn, Args&&... args)
        -> std::future<std::invoke_result_t<Fn, Args...>> {
        using ReturnT = std::invoke_result_t<Fn, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnT()>>(
            std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));

        std::future<ReturnT> result = task->get_future();
        {
            std::scoped_lock lock{mutex_};
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();

        return result;
    }

    [[nodiscard]] std::size_t workerCount() const noexcept;

private:
    void workerLoop();

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
    bool stop_{false};
};

} // namespace veys
