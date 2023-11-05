#pragma once

#include <mutex>
#include <thread>
#include <vector>
#include <list>
#include <functional>
#include <condition_variable>

namespace wolv::util {

        class ThreadPool {
        public:
            using Task = std::function<void(const std::atomic<bool> &)>;

            explicit ThreadPool(size_t threadCount) {
                for (size_t i = 0; i < threadCount; i += 1) {
                    this->m_threads.emplace_back([this]{
                        this->waitForTasks();
                    });
                }
            }

            ~ThreadPool() {
                this->stop();
            }

            ThreadPool(const ThreadPool &) = delete;
            ThreadPool(ThreadPool &&other) noexcept {
                if (this != &other) {
                    other.stop();
                    this->stop();

                    this->m_threads.clear();
                    this->m_stop = false;

                    for (size_t i = 0; i < other.m_threads.size(); i += 1) {
                        this->m_threads.emplace_back([this]{
                            this->waitForTasks();
                        });
                    }
                    other.m_threads.clear();

                    this->m_tasks = std::move(other.m_tasks);
                }
            }

            ThreadPool& operator=(const ThreadPool &) = delete;
            ThreadPool& operator=(ThreadPool &&other) noexcept {
                if (this != &other) {
                    other.stop();
                    this->stop();

                    this->m_threads.clear();
                    this->m_stop = false;

                    for (size_t i = 0; i < other.m_threads.size(); i += 1) {
                        this->m_threads.emplace_back([this]{
                            this->waitForTasks();
                        });
                    }

                    other.m_threads.clear();

                    this->m_tasks = std::move(other.m_tasks);
                }

                return *this;
            }

            void enqueue(Task &&task) {
                {
                    std::unique_lock lock(this->m_mutex);
                    this->m_tasks.emplace_back(task);
                }

                this->m_condition.notify_one();
            }

            void stop() {
                {
                    std::unique_lock lock(this->m_mutex);
                    this->m_stop = true;
                }

                this->m_condition.notify_all();

                for (auto &thread : this->m_threads) {
                    if (thread.joinable())
                        thread.detach();
                }
            }

        private:
            void waitForTasks() {
                while (true) {
                    Task task;
                    {
                        std::unique_lock lock(this->m_mutex);
                        this->m_condition.wait(lock, [this] {
                            return this->m_stop || !this->m_tasks.empty();
                        });

                        if (this->m_stop && this->m_tasks.empty())
                            return;

                        task = std::move(this->m_tasks.front());
                        this->m_tasks.pop_front();
                    }

                    task(this->m_stop);
                }
            }

        private:
            std::vector<std::thread> m_threads;
            std::list<Task> m_tasks;

            std::mutex m_mutex;
            std::condition_variable m_condition;
            std::atomic<bool> m_stop = false;
        };
}