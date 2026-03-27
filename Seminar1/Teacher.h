#ifndef TEACHER_H
#define TEACHER_H

#include "Person.h"

class Teacher : public Person {
public:
    explicit Teacher(const std::string& n) : Person(n) {}
    void print() const override;
};

#endif