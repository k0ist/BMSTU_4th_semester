#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <string>
#include <windows.h>

class TaskTimer {
    std::chrono::high_resolution_clock::time_point start_time;
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    long long stop() {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    }
};

void bubbleSort(std::vector<int>& arr) {
    size_t n = arr.size();
    for (size_t i = 0; i < n - 1; i++) {
        for (size_t j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) std::swap(arr[j], arr[j + 1]);
        }
    }
}

void insertionSort(std::vector<int>& arr) {
    size_t n = arr.size();
    for (size_t i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void merge(std::vector<int>& arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    std::vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(std::vector<int>& arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

void quickSort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        int pivot = arr[high];
        int i = (low - 1);
        for (int j = low; j <= high - 1; j++) {
            if (arr[j] < pivot) {
                i++;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[high]);
        int pi = i + 1;
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

std::vector<int> generateRandomVector(int size) {
    std::vector<int> v(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100000);
    for (int& x : v) x = dis(gen);
    return v;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    TaskTimer timer;
    const int N = 10000;

    auto baseData = generateRandomVector(N);
    std::vector<int> data;

    data = baseData;
    timer.start();
    std::sort(data.begin(), data.end());
    std::cout << "Время выполнения std::sort: " << timer.stop() << " миллисекунд" << std::endl;

    data = baseData;
    timer.start();
    quickSort(data, 0, N - 1);
    std::cout << "Время выполнения Quick Sort: " << timer.stop() << " миллисекунд" << std::endl;

    data = baseData;
    timer.start();
    mergeSort(data, 0, N - 1);
    std::cout << "Время выполнения Merge Sort: " << timer.stop() << " миллисекунд" << std::endl;

    data = baseData;
    timer.start();
    insertionSort(data);
    std::cout << "Время выполнения Insertion Sort: " << timer.stop() << " миллисекунд" << std::endl;

    data = baseData;
    timer.start();
    bubbleSort(data);
    std::cout << "Время выполнения Bubble Sort: " << timer.stop() << " миллисекунд" << std::endl;

    return 0;
}