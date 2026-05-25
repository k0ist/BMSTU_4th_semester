#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <cuda_runtime.h>
#include <windows.h>


void increaseBrightnessCPU(unsigned char* input, unsigned char* output, int width, int height, int delta) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int value = input[idx] + delta;
            output[idx] = (value > 255) ? 255 : value;
        }
    }
}

__global__ void increaseBrightness(unsigned char* input, unsigned char* output, int width, int height, int delta) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {
        int idx = y * width + x;
        int value = input[idx] + delta;
        output[idx] = (value > 255) ? 255 : value;
    }
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int width = 1024;
    int height = 1024;
    int N = width * height;
    size_t size = N * sizeof(unsigned char);
    int delta = 50;

    unsigned char* h_input = new unsigned char[N];
    unsigned char* h_output_CPU = new unsigned char[N];
    unsigned char* h_output_GPU = new unsigned char[N];

    for (int i = 0; i < N; i++) {
        h_input[i] = static_cast<unsigned char>(rand() % 256);
    }

    auto start_cpu = std::chrono::high_resolution_clock::now();
    increaseBrightnessCPU(h_input, h_output_CPU, width, height, delta);
    auto end_cpu = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_cpu = end_cpu - start_cpu;
    std::cout << "Время выполнения на CPU: " << time_cpu.count() << " мс" << std::endl;

    unsigned char *d_input, *d_output;
    cudaMalloc((void**)&d_input, size);
    cudaMalloc((void**)&d_output, size);

    cudaMemcpy(d_input, h_input, size, cudaMemcpyHostToDevice);

    dim3 threadsPerBlock(16, 16);
    dim3 blocksPerGrid((width + threadsPerBlock.x - 1) / threadsPerBlock.x,
                       (height + threadsPerBlock.y - 1) / threadsPerBlock.y);

    auto start_gpu = std::chrono::high_resolution_clock::now();

    increaseBrightness<<<blocksPerGrid, threadsPerBlock>>>(d_input, d_output, width, height, delta);
    cudaDeviceSynchronize();

    auto end_gpu = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_gpu = end_gpu - start_gpu;
    std::cout << "Время выполнения на GPU: " << time_gpu.count() << " мс" << std::endl;

    cudaMemcpy(h_output_GPU, d_output, size, cudaMemcpyDeviceToHost);

    bool success = true;
    for (int i = 0; i < N; i++) {
        if (h_output_CPU[i] != h_output_GPU[i]) {
            success = false;
            break;
        }
    }
    std::cout << "Результаты вычислений совпадают: " << (success ? "Да" : "Нет") << std::endl;

    cudaFree(d_input);
    cudaFree(d_output);
    delete[] h_input;
    delete[] h_output_CPU;
    delete[] h_output_GPU;

    return 0;
}