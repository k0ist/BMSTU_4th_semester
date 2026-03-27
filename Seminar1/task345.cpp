#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
#include <fstream>
#define VECTOR_ID_GRADES std::vector<std::pair<unsigned, std::vector<double>>>
#define VECTOR_ID_GRADE std::vector<std::pair<unsigned, double>>

#pragma pack(push,1)
struct Students_file_header {
    char signature[4];
    unsigned version;
    unsigned number_of_students;
};
#pragma pack(pop)

unsigned number_of_students();

unsigned number_of_subjects();

void grades_fill(VECTOR_ID_GRADES &, const unsigned &, const unsigned &);

VECTOR_ID_GRADE mean_grade_by_student(const VECTOR_ID_GRADES &);

VECTOR_ID_GRADE mean_grade_by_subject(const VECTOR_ID_GRADES &);

void print_VECTOR_ID_GRADES(const VECTOR_ID_GRADES &);

void print_VECTOR_ID_GRADE(const VECTOR_ID_GRADE &);

unsigned max_mean_student(const VECTOR_ID_GRADES &);

VECTOR_ID_GRADE sort_VECTOR_ID_GRADE(VECTOR_ID_GRADE,
                                     bool (*)(const std::pair<unsigned, double> &,
                                              const std::pair<unsigned, double> &) =
                                     [](const std::pair<unsigned, double> &a,
                                        const std::pair<unsigned, double> &b) -> bool {
                                         if (a.second > b.second) {
                                             return true;
                                         }
                                         if (a.second < b.second) {
                                             return false;
                                         }
                                         if (a.first < b.first) {
                                             return true;
                                         }
                                         if (a.first > b.first) {
                                             return false;
                                         }
                                         return true;
                                     });

VECTOR_ID_GRADE erase_by_threshold(VECTOR_ID_GRADE, const double &);

unsigned count_by_threshold(const VECTOR_ID_GRADE &, const double &, auto comparator);

void save_to_file(const std::string &title, const Students_file_header &header, VECTOR_ID_GRADES &mat);

VECTOR_ID_GRADES load_from_file(const std::string &title, const Students_file_header &header);

int main() {
    const unsigned N = number_of_students();
    const unsigned M = number_of_subjects();

    VECTOR_ID_GRADES all_grades;


    grades_fill(all_grades, N, M);
    std::cout << "Grades: " << std::endl;
    print_VECTOR_ID_GRADES(all_grades);

    const VECTOR_ID_GRADE mean_grades = mean_grade_by_student(all_grades);
    std::cout << "Means by student: " << std::endl;
    print_VECTOR_ID_GRADE(mean_grades);

    std::cout << "Means by subject: " << std::endl;
    print_VECTOR_ID_GRADE(mean_grade_by_subject(all_grades));

    std::cout << "Student with the highest mean grade: " << max_mean_student(all_grades) << std::endl;

    std::cout << "Sorted by grades: " << std::endl;
    print_VECTOR_ID_GRADE(sort_VECTOR_ID_GRADE(mean_grades));

    double threshold;
    std::cout << "Enter grade threshold: " << std::endl;
    std::cin >> threshold;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Erased by threshold: " << std::endl;
    print_VECTOR_ID_GRADE(erase_by_threshold(mean_grades, threshold));


    std::cout << "Count of excellent students: " << count_by_threshold(mean_grades, 4.5, std::greater_equal<double>())
              << std::endl;

    std::cout << "Count of near to expulsion students: " << count_by_threshold(mean_grades, 3, std::less<double>()) <<
              std::endl;

    const Students_file_header header{{'s', 't', 'u', 'd'}, 1, N};
    std::cout << "Size of header: " << sizeof(header) << std::endl;

    save_to_file("Grades.stud", header, all_grades);

    VECTOR_ID_GRADES all_grades_from_file = load_from_file("Grades.stud", header);
    std::cout << "Grades from file: " << std::endl;
    print_VECTOR_ID_GRADES(all_grades_from_file);
}

unsigned number_of_students() {
    int N;
    std::cout << "Enter number of students: " << std::endl;
    while (true) {
        std::cin >> N;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (N <= 0) {
            std::cerr << "Invalid number of students! It must be greater then 0!" << std::endl;
        } else {
            break;
        }
    }
    return static_cast<unsigned>(N);
}

unsigned number_of_subjects() {
    int M;
    std::cout << "Enter number of subjects: " << std::endl;
    while (true) {
        std::cin >> M;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (M <= 0) {
            std::cerr << "Invalid number of subjects! It must be greater then 0!" << std::endl;
        } else {
            break;
        }
    }
    return static_cast<unsigned>(M);
}

void grades_fill(VECTOR_ID_GRADES &mat, const unsigned &N, const unsigned &M) {
    unsigned i = 0;
    while (i < N) {
        std::vector<double> grades;
        unsigned j = 0;
        while (j < M) {
            std::cout << "Enter grade of student number " << i << " on subject " << j << ":" << std::endl;
            double grade;
            std::cin >> grade;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (grade < 0 || 5 < grade) {
                std::cerr << "Invalid grade! It must be in [0, 5]!" << std::endl;
            } else {
                grades.push_back(grade);
                j++;
            }
        }
        mat.emplace_back(i, grades);
        i++;
    }
}

VECTOR_ID_GRADE mean_grade_by_student(const VECTOR_ID_GRADES &mat) {
    VECTOR_ID_GRADE means;
    const unsigned N = mat.size();
    const unsigned M = mat[0].second.size();
    for (unsigned i = 0; i < N; ++i) {
        double sum = 0;
        for (unsigned j = 0; j < M; ++j) {
            sum += mat[i].second[j];
        }
        means.emplace_back(i, sum / M);
    }
    return means;
}

VECTOR_ID_GRADE mean_grade_by_subject(const VECTOR_ID_GRADES &mat) {
    VECTOR_ID_GRADE means;
    const unsigned N = mat.size();
    const unsigned M = mat[0].second.size();
    for (unsigned i = 0; i < M; ++i) {
        double sum = 0;
        for (unsigned j = 0; j < N; ++j) {
            sum += mat[j].second[i];
        }
        means.emplace_back(i, sum / N);
    }
    return means;
}

void print_VECTOR_ID_GRADES(const VECTOR_ID_GRADES &mat) {
    for (const auto &[id, grades]: mat) {
        std::cout << "Student " << id << ": ";
        for (const auto &elm: grades) {
            std::cout << elm << " ";
        }
        std::cout << std::endl;
    }
}

void print_VECTOR_ID_GRADE(const VECTOR_ID_GRADE &vec) {
    for (const auto &[id, grade]: vec) {
        std::cout << id << ": " << grade << std::endl;
    }
}

unsigned max_mean_student(const VECTOR_ID_GRADES &mat) {
    const VECTOR_ID_GRADE means = mean_grade_by_student(mat);
    double max = means[0].second;
    unsigned max_idx = means[0].first;
    for (const auto &[id, grade]: means) {
        if (max < grade) {
            max = grade;
            max_idx = id;
        }
    }
    return max_idx;
}

VECTOR_ID_GRADE sort_VECTOR_ID_GRADE(
        VECTOR_ID_GRADE vec,
        bool (*comparator)(const std::pair<unsigned, double> &, const std::pair<unsigned, double> &)) {
    std::sort(vec.begin(), vec.end(), comparator);
    return vec;
}

VECTOR_ID_GRADE erase_by_threshold(VECTOR_ID_GRADE vec, const double &threshold) {
    vec.erase(std::remove_if(vec.begin(), vec.end(), [&threshold](const std::pair<unsigned, double> &a) -> bool {
        return a.second < threshold;
    }), vec.end());
    return vec;
}

unsigned count_by_threshold(const VECTOR_ID_GRADE &vec, const double &threshold, auto comparator) {
    unsigned counter = 0;
    for (const auto &[id, grade]: vec) {
        if (comparator(grade, threshold)) {
            counter++;
        }
    }
    return counter;
}

void save_to_file(const std::string &title, const Students_file_header &header, VECTOR_ID_GRADES &mat) {
    std::ofstream file(title, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for writing!" << std::endl;
    } else {
        file.write(reinterpret_cast<const char *>(&header), sizeof(header));
        const unsigned M = mat[0].second.size();
        file.write(reinterpret_cast<const char *>(&M), sizeof(M));
        for (const auto &[id, grades]: mat) {
            for (const auto &grade: grades) {
                file.write(reinterpret_cast<const char *>(&grade), sizeof(grade));
            }
        }
    }
    file.close();
}

VECTOR_ID_GRADES load_from_file(const std::string &title, const Students_file_header &header) {
    std::ifstream file(title, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for reading!" << std::endl;
    } else {
        Students_file_header file_header{};
        file.read(reinterpret_cast<char *>(&file_header), sizeof(file_header));
        if (!file || !std::equal(std::begin(file_header.signature), std::end(file_header.signature),
                                 std::begin(header.signature)) || file_header.version != header.version) {
            std::cerr << "Wrong signature or version of file!" << std::endl;
            file.close();
        } else {
            const unsigned N = file_header.number_of_students;
            unsigned M;
            file.read(reinterpret_cast<char *>(&M), sizeof(M));
            VECTOR_ID_GRADES mat;
            for (unsigned i = 0; i < N && file; ++i) {
                std::vector<double> grades;
                for (unsigned j = 0; j < M && file; ++j) {
                    double grade;
                    file.read(reinterpret_cast<char *>(&grade), sizeof(grade));
                    grades.push_back(grade);
                }
                mat.emplace_back(i, grades);
            }
            return mat;
        }
    }
    file.close();
}