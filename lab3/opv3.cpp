#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <chrono>
#include <string>
#include <locale>
#include <atomic>
#include <random>
#include <iomanip>

using namespace std;

atomic<int> task_completed(0);
atomic<int> task_started(0);

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

void runTask(int id, int value, promise<string>&& res_promise) {
    cout << "   [ПОТОК " << this_thread::get_id() << "] Задача " << id
        << " (F(" << value << ")) начала выполняться " << endl;
    task_started++;

    packaged_task<string()> task([id, value]() -> string {
        long long fib = fibonacci(value);
        return "Задача " + to_string(id) + ": F(" + to_string(value) + ") = " + to_string(fib);
        });

    future<string> f = task.get_future();

    task();

    cout << "   [ПОТОК " << this_thread::get_id() << "] Задача " << id
        << " завершила вычисления, ожидаю get() ;);)" << endl;

    string result = f.get();

    cout << "   [ПОТОК " << this_thread::get_id() << "] Задача " << id
        << " получила результат через get()" << endl;

    res_promise.set_value(result);

    task_completed++;

    cout << "   [ПОТОК " << this_thread::get_id() << "] Задача " << id
        << " ЗАВЕРШЕНА. Выполнено задач: " << task_completed.load() << "/"
        << task_started.load() << endl;
}

int main() {
    setlocale(LC_ALL, "Russian");

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> n_dist(3, 15);
    uniform_int_distribution<> value_dist(1, 30);

    int N = n_dist(gen);

    cout << " Сгенерировано количество задач: " << N << endl;
    cout << endl;

    vector<int> values(N);
    cout << " Сгенерированные числа:" << endl;
    for (int i = 0; i < N; i++) {
        values[i] = value_dist(gen);
        cout << "            Задача " << i << ": вычисление F(" << values[i] << ")" << endl;
    }
    cout << endl;

    vector<promise<string>> promises(N);
    vector<future<string>> futures;

    for (int i = 0; i < N; i++) {
        futures.push_back(promises[i].get_future());
    }

    vector<thread> threads;
    for (int i = 0; i < N; i++) {
        threads.emplace_back(runTask, i, values[i], move(promises[i]));
        this_thread::sleep_for(chrono::milliseconds(50)); 
    }

    promise<void> monitor_promise;
    future<void> monitor_future = monitor_promise.get_future();

    thread monitor([&monitor_promise, N]() {
        cout << "[МОНИТОР] Запущен. Отслеживаю выполнение задач..." << endl;

        int last_completed = 0;
        while (task_completed.load() < N) {
            int current = task_completed.load();
            if (current != last_completed) {
                cout << "[МОНИТОР] Прогресс: выполнено " << current << " из " << N << " задач" << endl;
                last_completed = current;
            }
            this_thread::sleep_for(chrono::milliseconds(300));
        }

        cout << "[МОНИТОР] Все " << N << " задач выполнены!" << endl;

        monitor_promise.set_value();
        });

    cout << "[ОСНОВНОЙ ПОТОК] Ожидаю сигнал от монитора" << endl;
    cout << endl;

    auto start_wait = chrono::steady_clock::now();
    monitor_future.wait(); 
    auto end_wait = chrono::steady_clock::now();

    auto wait_time = chrono::duration_cast<chrono::milliseconds>(end_wait - start_wait);

    cout << endl;
    cout << "[ОСНОВНОЙ ПОТОК] Получил сигнал о завершении всех задач!" << endl;
    cout << endl;


    for (int i = 0; i < N; i++) {
        cout << "[ОСНОВНОЙ ПОТОК] Вызов future.get() для задачи " << i << " (блок ожидание)" << endl;
        auto start_get = chrono::steady_clock::now();
        string result = futures[i].get(); 
        auto end_get = chrono::steady_clock::now();
        auto get_time = chrono::duration_cast<chrono::milliseconds>(end_get - start_get);
        cout << "[ОСНОВНОЙ ПОТОК] Результат получен: " << result << endl;
        cout << endl;
    }

    cout << " ... Ожидаю завершения всех потоков ... ;-) " << endl;
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    monitor.join();

    cout << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "  - Всего задач: " << N << endl;
    cout << "  - Всего потоков: " << N + 2 << " (" << N << " рабочих + 1 монитор + 1 основной)" << endl;
    cout << endl;

    return 0;
}