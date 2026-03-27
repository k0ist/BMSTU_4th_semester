#include "Group.h"
#include "Student.h"
#include <algorithm>

Group::Group(std::string name) : groupName(std::move(name)) {}

void Group::addStudent(Student* s) {
    students.push_back(s);
}

void Group::removeStudent(const std::string& name) {
    students.erase(std::remove_if(students.begin(), students.end(),
                                  [&name](Student* s) { return s->getName() == name; }), students.end());
}

double Group::getGroupAverage() const {
    if (students.empty()) return 0.0;
    double sum = 0;
    for (auto s : students) sum += s->getAverageScore();
    return sum / students.size();
}

const std::vector<Student*>& Group::getStudents() const {
    return students;
}