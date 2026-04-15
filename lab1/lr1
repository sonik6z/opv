#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void printNumbers(const string& name, int start, int end) {
    for (int i = start; i <= end; ++i) {
        cout << "Поток " << name << ": " << i << endl;
        this_thread::sleep_for(chrono::milliseconds(300));
    }
    cout << "Поток " << name << " завершил работу" << endl;
}

int main() {
    setlocale(LC_ALL, "Russian");
    cout << "Основной поток - запуск" << endl;

    thread thread1(printNumbers, "A", 1, 5);
    thread thread2(printNumbers, "B", 1, 5);

    // основной поток 
    for (int i = 1; i <= 3; ++i) {
        this_thread::sleep_for(chrono::milliseconds(150));
        cout << "Основной поток - работаем работаем" << i << endl;
    }

    thread1.join();
    thread2.join();

    cout << "Основной поток - завершено" << endl;
    return 0;
}
