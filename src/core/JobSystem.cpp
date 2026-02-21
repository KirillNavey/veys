#include "core/JobSystem.hpp"

#include <algorithm>

namespace veys {

JobSystem::JobSystem() {
    const unsigned hw = std::thread::hardware_concurrency();
    const std::size_t count = std::max(1u, hw == 0 ? 1u : hw);
    workers_.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        workers_.emplace_back([this]() { workerLoop(); });
    }
}

JobSystem::~JobSystem() {
    {
        std::scoped_lock lock{mutex_};
        stop_ = true;
    }
    cv_.notify_all();

    for (auto& w : workers_) {
        if (w.joinable()) {
            w.join();
        }
    }
}

std::size_t JobSystem::workerCount() const noexcept {
    return workers_.size();
}

void JobSystem::workerLoop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock lock{mutex_};
            cv_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty()) {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        task();
    }
}

} // namespace veys
