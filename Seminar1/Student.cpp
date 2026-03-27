#include "Student.h"

Student::Student(const std::string& n) : Person(n), recordBook() {}

Student::Student(const std::string& n, const std::string& bookNum, const std::vector<int>& g)
        : Person(n), recordBook(bookNum, g) {}

void Student::print() const {
    std::cout << "[Student] ";
    Person::print();
    std::cout << "GPA: " << getAverageScore() << std::endl;
}

double Student::getAverageScore() const {
    return recordBook.getAverage();
}

void Student::addGrade(int g) {
    recordBook.addGrade(g);
}

const RecordBook& Student::getRecordBook() const {
    return recordBook;
}