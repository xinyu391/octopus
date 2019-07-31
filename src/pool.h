#ifndef OCTOPUS_POOL
#define OCTOPUS_POOL

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace octopus {

class Pool {
public:
    Pool(int pool_size)
        : poolSize(pool_size)
        , queueSize(-1)
        , idle_thread_count(0)
        , quit(false){

        };
    Pool(int pool_size, int queue_size)
        : poolSize(pool_size)
        , queueSize(queue_size)
        , idle_thread_count(0)
        , quit(false){};

    template <class Fn, class... Args>
    auto enqueue(Fn&& f, Args&&... args) -> std::future<typename std::result_of<Fn(Args...)>::type> {
        std::unique_lock<std::mutex> lck(mtx);
        while (queueSize > 0 && queue.size() >= queueSize) {
            codv_que.wait(lck);
        }
        using return_type = typename std::result_of<Fn(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<Fn>(f), std::forward<Args>(args)...));
        std::future<return_type> ret = task->get_future();
        // check thread add to queue,
        queue.emplace([task]() { (*task)(); });
        if (threads.size() == poolSize) { // notify one
            codv.notify_one();
        } else {
            if (idle_thread_count < 1) {
                std::thread t([this]() {
                    while (1) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lck(this->mtx);
                            idle_thread_count++;
                            this->codv.wait(lck,
                                [this] { return this->quit || !this->queue.empty(); });
                            if (this->quit && this->queue.empty())
                                return;
                            idle_thread_count--;
                            task = std::move(this->queue.front());
                            this->queue.pop();
                            if (queueSize > 0) {
                                this->codv_que.notify_all();
                            }
                        }
                        task();
                    }
                });
                threads.push_back(std::move(t));
            } else {
                codv.notify_one();
            }
        }
        return ret;
    };

    //  wait all thread  quit friendly.
    ~Pool() {
        {
            std::unique_lock<std::mutex> lck(mtx);
            quit = true;
        }
        if (queueSize > 0) {
            this->codv_que.notify_all();
        }
        codv.notify_all(); // 唤醒线程池所有线程

        for (auto& t : threads) {
            t.join();
        }
    };

public:
    std::mutex mtx;
    std::queue<std::function<void()>> queue;
    std::condition_variable codv; // 唤醒线程池
    std::condition_variable codv_que; // 唤醒add 等待队列
    bool quit;

private:
    int poolSize;
    int queueSize;
    int idle_thread_count;
    std::vector<std::thread> threads;
};
}
#endif