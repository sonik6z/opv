#include <iostream>
#include <chrono>
#include <omp.h>

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");

    const long long N = 10000000;  // 10^7 элементов
    int* arr = new int[N];

    cout << "Массив заполняется" << endl;
    for (long long i = 0; i < N; i++) {
        arr[i] = i % 10;  // разные числа
    }
    cout << "Массив заполнен" << endl;

    cout << "\nПоследовательно: " << endl;
    auto start = chrono::high_resolution_clock::now();

    long long summamas = 0;
    for (long long i = 0; i < N; i++) {
        summamas += arr[i];
    }

    auto end = chrono::high_resolution_clock::now();
    auto vremya = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Сумма = " << summamas << endl;
    cout << "Время = " << vremya.count() << " ms" << endl;

    cout << "\nПараллельно: " << endl;
    start = chrono::high_resolution_clock::now();

    long long sum_par = 0;

#pragma omp parallel for reduction(+:sum_par)
    for (long long i = 0; i < N; i++) {
        sum_par += arr[i];
    }

    end = chrono::high_resolution_clock::now();
    auto time_par = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Сумма = " << sum_par << endl;
    cout << "Время = " << time_par.count() << " ms" << endl;
    cout << "Ускорение = " << (double)vremya.count() / time_par.count() << "x" << endl;


    delete[] arr;
    return 0;
}