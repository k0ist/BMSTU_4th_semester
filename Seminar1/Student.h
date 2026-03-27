#ifndef STUDENT_H
#define STUDENT_H

#include "Person.h"
#include "RecordBook.h"

class Student : public Person {
private:
    RecordBook recordBook;
public:
    explicit Student(const std::string& n);
    Student(const std::string& n, const std::string& bookNum, const std::vector<int>& g);

    void print() const override;
    double getAverageScore() const;
    void addGrade(int g);
    const RecordBook& getRecordBook() const;
};

#endif