#include "RecordBook.h"
#include <numeric>

RecordBook::RecordBook() : bookNumber("000000") {}

RecordBook::RecordBook(std::string id, std::vector<int> g)
        : bookNumber(std::move(id)), grades(std::move(g)) {}

double RecordBook::getAverage() const {
    if (grades.empty()) return 0.0;
    double sum = std::accumulate(grades.begin(), grades.end(), 0.0);
    return sum / grades.size();
}

void RecordBook::addGrade(int grade) {
    grades.push_back(grade);
}

const std::vector<int>& RecordBook::getGrades() const {
    return grades;
}

std::string RecordBook::getNumber() const {
    return bookNumber;
}