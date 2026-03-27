#include <iostream>


void mean(double *grades, int n) {
    double sum = 0;

    for (int i = 0; i < n; ++i) {
        sum += grades[i];
    }

    std::cout << "Average value of array: " << sum / n << "\n";
}

void max(double *grades, int n) {
    double m = 0;

    for (int i  = 0; i < n; ++i) {
        if (grades[i] > m) {
            m = grades[i];
        }
    }

    std::cout << "Maximum of array: " << m << "\n";
}


void min(double *grades, int n) {
    double m = 10e6;

    for (int i  = 0; i < n; ++i) {
        if (grades[i] < m) {
            m = grades[i];
        }
    }

    std::cout << "Minimum of array: " << m << "\n";
}


void mvg(double MVG, double *grades, int n) {
    int counter = 0;

    for (int i = 0; i < n; ++i) {
        if (grades[i] >= MVG) counter++;
    }

    std::cout << "Number of students those have done the test correctly: " << counter << "\n";
}

int main() {
    int n;
    std::cin >> n;
    double *grades = new double[n];

    for (int i = 0; i < n; ++i) {
        std::cin >> grades[i];
    }

    double MVG;
    std::cin >> MVG;

    mean(grades, n);
    max(grades, n);
    min(grades, n);
    mvg(MVG, grades, n);

    delete[] grades;

    return 0;
}
