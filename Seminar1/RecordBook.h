#ifndef RECORDBOOK_H
#define RECORDBOOK_H

#include <vector>
#include <string>

class RecordBook {
private:
    std::string bookNumber;
    std::vector<int> grades;
public:
    RecordBook();
    RecordBook(std::string id, std::vector<int> g);

    double getAverage() const;
    void addGrade(int grade);
    const std::vector<int>& getGrades() const;
    std::string getNumber() const;
};

#endif