#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

const char* SHM_NAME = "Local\\BmstuShmBarrier";
const char* SEM_NAME = "Local\\BmstuSemBarrier";
const int NUM_CHILDREN = 5;

struct SharedData {
    int results[NUM_CHILDREN];
};

void run_child_logic(int child_index) {
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHM_NAME);
    if (hMapFile == NULL) {
        std::cerr << "[Дочерний " << child_index << "] Ошибка OpenFileMapping: " << GetLastError() << std::endl;
        return;
    }

    SharedData* pData = (SharedData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
    if (pData == NULL) {
        std::cerr << "[Дочерний " << child_index << "] Ошибка MapViewOfFile: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        return;
    }

    HANDLE hSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEM_NAME);
    if (hSemaphore == NULL) {
        std::cerr << "[Дочерний " << child_index << "] Ошибка OpenSemaphore: " << GetLastError() << std::endl;
        UnmapViewOfFile(pData);
        CloseHandle(hMapFile);
        return;
    }

    std::cout << "[Дочерний " << child_index << "] Начал вычисления..." << std::endl;

    Sleep((1 + child_index) * 1000);

    pData->results[child_index] = (child_index + 1) * 111;

    std::cout << "[Дочерний " << child_index << "] Достиг барьера. Сигнализирую родителю." << std::endl;

    ReleaseSemaphore(hSemaphore, 1, NULL);

    UnmapViewOfFile(pData);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);
}

void run_parent_logic(const std::string& program_path) {
    HANDLE hMapFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(SharedData),
            SHM_NAME
    );

    if (hMapFile == NULL) {
        std::cerr << "[Родитель] Ошибка CreateFileMapping: " << GetLastError() << std::endl;
        return;
    }

    SharedData* pData = (SharedData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
    if (pData == NULL) {
        std::cerr << "[Родитель] Ошибка MapViewOfFile: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        return;
    }

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        pData->results[i] = 0;
    }

    HANDLE hSemaphore = CreateSemaphoreA(NULL, 0, NUM_CHILDREN, SEM_NAME);
    if (hSemaphore == NULL) {
        std::cerr << "[Родитель] Ошибка CreateSemaphore: " << GetLastError() << std::endl;
        UnmapViewOfFile(pData);
        CloseHandle(hMapFile);
        return;
    }

    std::cout << "[Родитель] Системные объекты созданы. Запуск дочерних процессов..." << std::endl;

    std::vector<PROCESS_INFORMATION> pi_list(NUM_CHILDREN);

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi_list[i], sizeof(PROCESS_INFORMATION));

        std::string cmd = program_path + " " + std::to_string(i);
        std::vector<char> cmd_chars(cmd.begin(), cmd.end());
        cmd_chars.push_back('\0');

        if (!CreateProcessA(
                NULL,
                cmd_chars.data(),
                NULL,
                NULL,
                FALSE,
                0,
                NULL,
                NULL,
                &si,
                &pi_list[i]
        ))
        {
            std::cerr << "[Родитель] Ошибка CreateProcess для индекса " << i << ": " << GetLastError() << std::endl;
        }
    }

    std::cout << "[Родитель] Ожидаю у барьера, пока все " << NUM_CHILDREN << " процессов завершат работу..." << std::endl;

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        WaitForSingleObject(hSemaphore, INFINITE);
    }

    std::cout << "\n[Родитель] БАРЬЕР ПРОЙДЕН! Все дочерние процессы прислали сигнал." << std::endl;
    std::cout << "[Родитель] Чтение результатов из Shared Memory:" << std::endl;

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        std::cout << "  -> Результат от процесса [" << i << "]: " << pData->results[i] << std::endl;
    }

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        CloseHandle(pi_list[i].hProcess);
        CloseHandle(pi_list[i].hThread);
    }

    UnmapViewOfFile(pData);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);

    std::cout << "\n[Родитель] Все ресурсы Windows очищены. Завершение работы." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int child_index = std::stoi(argv[1]);
        run_child_logic(child_index);
        return 0;
    }
    else {
        run_parent_logic(argv[0]);
        return 0;
    }
}