#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <iostream>

class Person {
protected:
    std::string name;
public:
    explicit Person(const std::string& n) : name(n) {}
    virtual ~Person() = default;
    virtual void print() const {
        std::cout << "Name: " << name << std::endl;
    }
    inline std::string getName() const { return name; }
};

#endif