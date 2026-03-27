#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <vector>
#include <string>
#include <cstdint>

class Student;

#pragma pack(push, 1)
struct FileHeader {
    char signature[4];
    uint32_t version;
    uint32_t studentCount;
};
#pragma pack(pop)

class FileManager {
public:
    static void saveBinary(const std::string& filename, const std::vector<Student*>& students);
    static void loadBinary(const std::string& filename);
};

#endif