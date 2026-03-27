#include "FileManager.h"
#include "Student.h"
#include <fstream>
#include <iostream>
#include <cstring>

void FileManager::saveBinary(const std::string& filename, const std::vector<Student*>& students) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) return;

    FileHeader header;
    std::memcpy(header.signature, "STDT", 4);
    header.version = 1;
    header.studentCount = static_cast<uint32_t>(students.size());

    std::cout << "Header size: " << sizeof(FileHeader) << " bytes" << std::endl;

    ofs.write(reinterpret_cast<char*>(&header), sizeof(header));

    for (auto s : students) {
        const auto& grades = s->getRecordBook().getGrades();
        uint32_t gradeCount = static_cast<uint32_t>(grades.size());
        ofs.write(reinterpret_cast<char*>(&gradeCount), sizeof(gradeCount));
        ofs.write(reinterpret_cast<const char*>(grades.data()), gradeCount * sizeof(int));
    }
}

void FileManager::loadBinary(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) return;

    FileHeader header;
    ifs.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (std::strncmp(header.signature, "STDT", 4) != 0) {
        std::cerr << "Error: Invalid file signature!" << std::endl;
        return;
    }

    std::cout << "Successfully read header. Version: " << header.version
              << ", Students: " << header.studentCount << std::endl;
}