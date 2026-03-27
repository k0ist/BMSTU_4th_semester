#include <iostream>
#include <vector>
#include <windows.h>


void average_student_mark(std::vector<std::vector<double>> grades, int student, int m) {
    double mean = 0;

    for (int i = 0; i < m; ++i) {
        mean += grades[student][i];
    }

    std::cout << "Средний балл студента " << student << " за все предметы: " << mean / m << "\n";
}


void average_subject_mark(std::vector<std::vector<double>> grades, int n, int subject) {
    double mean = 0;

    for (int i = 0; i < n; ++i) {
        mean += grades[i][subject];
    }

    std::cout << "Средний балл по предмету " << subject << ": " << mean / n << "\n";
}


void top_student(std::vector<std::vector<double>> grades, int n, int m) {
    double mean, maxAverage, top = 0;

    for (int i = 0; i < n; ++i) {
        double average = 0;

        for (int j = 0; j < m; ++j) {
            average += grades[i][j];
        }

        if (average / m > maxAverage) {
            maxAverage = average / m;
            top = i;
        }
    }

    std::cout << "Лучший студент " << top << " со средним баллом " << maxAverage << "\n";
}


int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    int n, m;
    std::cout << "Введите количество студентов и количество предметов: ";
    if (!(std::cin >> n >> m) || n <= 0 || m <= 0) {
        std::cout << "Некорректные параметры.";
        return 1;
    }

    std::vector<std::vector<double>> grades(n, std::vector<double>(m));

    for (int i = 0; i < n; ++i) {
        std::cout << "Введите оценки студента " << i << "\n";
        for (int j = 0; j < m; ++j) {
            double grade;
            while (true) {
                if (std::cin >> grade && grade >= 0 && grade <= 5) {
                    grades[i][j] = grade;
                    break;
                }
                std::cout << "Ошибка ввода";
                std::cin.clear();
                std::cin.ignore();
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        average_student_mark(grades, i, m);
    }

    for (int i = 0; i < n; ++i) {
        average_subject_mark(grades, n, i);
    }

    top_student(grades, n, m);
}