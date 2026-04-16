#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <chrono>
#include <locale>
#include <random>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

using namespace std;

class ThreadPool {
private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    mutex queue_mtx;
    condition_variable condition;
    bool happyend;

public:
    explicit ThreadPool(size_t num_threads) : happyend(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    function<void()> task;
                    {
                        unique_lock<mutex> lock(queue_mtx);
                        condition.wait(lock, [this] {
                            return happyend || !tasks.empty();
                            });

                        if (happyend && tasks.empty())
                            return;

                        task = move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
                });
        }
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> future<typename result_of<F(Args...)>::type> {

        using return_type = typename result_of<F(Args...)>::type;

        auto task = make_shared<packaged_task<return_type()>>(
            bind(forward<F>(f), forward<Args>(args)...)
        );

        future<return_type> result = task->get_future();

        {
            unique_lock<mutex> lock(queue_mtx);
            tasks.emplace([task]() { (*task)(); });
        }

        condition.notify_one();
        return result;
    }

    ~ThreadPool() {
        {
            unique_lock<mutex> lock(queue_mtx);
            happyend = true;
        }
        condition.notify_all();
        for (thread& worker : workers) {
            if (worker.joinable())
                worker.join();
        }
    }
};


long long fibonacci(int n) {
    if (n <= 1) return n;
    long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        long long next = a + b;
        a = b;
        b = next;
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    return b;
}


long long runTask(int id, int value) {
    cout << "   [ПОТОК " << this_thread::get_id() << "] Задача " << id
        << " (F(" << value << ")) начала выполняться" << endl;

    long long fib = fibonacci(value);

    cout << "   [ПОТОК " << this_thread::get_id() << "] Задача " << id
        << " завершила вычисления" << endl;

    cout << "   [ПОТОК " << this_thread::get_id() << "] Задача " << id
        << " завершена" << endl;

    return fib;
}

int main() {
    setlocale(LC_ALL, "Russian");

    ThreadPool pool(4);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> value_dist(10, 30);

    int N = 20;

    vector<int> values(N);
    for (int i = 0; i < N; i++) {
        values[i] = value_dist(gen);
        cout << "Задача " << i << ": F(" << values[i] << ")" << endl;
    }
    cout << endl;

    vector<future<long long>> futures;

    for (int i = 0; i < N; i++) {
        futures.push_back(pool.enqueue(runTask, i, values[i]));
    }


    for (int i = 0; i < N; i++) {
        cout << "Ожидание результата задачи " << i << endl;
        long long res = futures[i].get();
        cout << "Задача " << i << ": F(" << values[i] << ") = " << res << endl;
    }

    cout << " ------------- ВСЕ 20 ЗАДАЧ ВЫПОЛНЕНЫ! -------------------" << endl;


    return 0;
}