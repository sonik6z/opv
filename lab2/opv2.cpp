#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <vector>
using namespace std;

queue<int> zadachi;
mutex mtx;
recursive_mutex mtx2;
condition_variable cv;
bool happyend = false;

// прямое логирование
void log_simple(string text) {
    lock_guard<recursive_mutex> lock(mtx2);
    cout << "[" << this_thread::get_id() << "] " << text << endl;
}

// рекурсивный мьютекс вложенный вызов
void log_nested(string text) {
    lock_guard<recursive_mutex> lock(mtx2);
    cout << "[LOG] " << text << endl;
    log_simple("  Вложенный вызов " + text);  
}

void proizvoditel() {
    log_nested("Производитель начал");

    for (int i = 1; i <= 20; i++) {
        this_thread::sleep_for(chrono::milliseconds(50));
        lock_guard<mutex> lock(mtx);
        zadachi.push(i);
        cout << "Добавлено число - " << i << endl;
    }

    {
        lock_guard<mutex> lock(mtx);
        happyend = true;
    }
    cv.notify_all();

    log_nested("Производитель законичил");
}

void potrebitel(int id) {
    thread_local int count = 0;

    cout << "Потребитель" << id << " начал" << endl;

    while (true) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [] {
            return !zadachi.empty() || happyend;
            });

        if (zadachi.empty() && happyend) {
            break;
        }

        int chislo = zadachi.front();
        zadachi.pop();
        lock.unlock();

        int kvadrat = chislo * chislo;
        this_thread::sleep_for(chrono::milliseconds(100));

        cout << "Потребитель " << id
            << " (id = " << this_thread::get_id() << ")"
            << " вызвал число " << chislo
            << ", квадрат = " << kvadrat << endl;

        count++;
    }

    cout << "\n Потребитель " << id << endl;
    cout << "Кол-во обработанных задач - " << count << endl;
    cout << "id потока - " << this_thread::get_id() << endl;
    cout << "Потребитель" << id << " закончил" << endl;

    cout << "-------------------------------------------" << endl;

}

int main() {
    setlocale(LC_ALL, "Russian");

    cout << "Главный поток id " << this_thread::get_id() << "\n" << endl;

    cout << "Рекурсивный мьютекс" << endl;
    log_simple("Внешний вызов");
    log_simple("Обычный вызов");

    thread t1(proizvoditel);
    thread t2(potrebitel, 1);
    thread t3(potrebitel, 2);
    thread t4(potrebitel, 3);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    return 0;
}