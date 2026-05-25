#include <iostream>
#include <cmath>
#include <chrono>
#include <cuda_runtime.h>
#include <windows.h>


void vectorAddCPU(const float* A, const float* B, float* C, int N) {
    for (int i = 0; i < N; i++) {
        C[i] = A[i] + B[i];
    }
}

__global__ void vectorAddCUDA(const float* A, const float* B, float* C, int N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        C[idx] = A[idx] + B[idx];
    }
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    int N = 1000000;
    size_t size = N * sizeof(float);

    float* h_A = new float[N];
    float* h_B = new float[N];
    float* h_C_CPU = new float[N];
    float* h_C_GPU = new float[N];

    for (int i = 0; i < N; i++) {
        h_A[i] = static_cast<float>(i) * 0.001f;
        h_B[i] = static_cast<float>(i) * 0.002f;
    }

    auto start_cpu = std::chrono::high_resolution_clock::now();
    vectorAddCPU(h_A, h_B, h_C_CPU, N);
    auto end_cpu = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_cpu = end_cpu - start_cpu;
    std::cout << "Время выполнения на CPU: " << time_cpu.count() << " мс" << std::endl;

    float *d_A, *d_B, *d_C;
    cudaMalloc((void**)&d_A, size);
    cudaMalloc((void**)&d_B, size);
    cudaMalloc((void**)&d_C, size);

    cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, size, cudaMemcpyHostToDevice);

    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

    auto start_gpu = std::chrono::high_resolution_clock::now();

    vectorAddCUDA<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C, N);
    cudaDeviceSynchronize();

    auto end_gpu = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_gpu = end_gpu - start_gpu;
    std::cout << "Время выполнения на GPU: " << time_gpu.count() << " мс" << std::endl;

    cudaMemcpy(h_C_GPU, d_C, size, cudaMemcpyDeviceToHost);

    bool success = true;
    for (int i = 0; i < N; i++) {
        if (std::abs(h_C_CPU[i] - h_C_GPU[i]) > 1e-4) {
            success = false;
            break;
        }
    }
    std::cout << "Результаты вычислений совпадают: " << (success ? "Да" : "Нет") << std::endl;

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    delete[] h_A;
    delete[] h_B;
    delete[] h_C_CPU;
    delete[] h_C_GPU;

    return 0;
}