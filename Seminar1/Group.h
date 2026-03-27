#ifndef GROUP_HPP
#define GROUP_HPP

#include <vector>
#include <string>

class Student; // Forward declaration

class Group {
private:
    std::string groupName;
    std::vector<Student*> students;
public:
    explicit Group(std::string name);
    void addStudent(Student* s);
    void removeStudent(const std::string& name);
    double getGroupAverage() const;
    const std::vector<Student*>& getStudents() const;
};

#endif