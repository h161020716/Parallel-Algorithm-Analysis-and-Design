#include <iostream>
#include <pthread.h>
#include <vector>
#include <fstream>
#include <climits>
#include <cfloat> 
#include <chrono>
#include <windows.h>
#include <algorithm>
#include <thread>

using namespace std;
using namespace chrono;

vector<double> vec, sortedVec;
pthread_barrier_t barrier;
const int THREADS_NUM = 24;
const int CPU_CORES = 24;

class Section {
    public:
        int left;
        int right;
        Section() = default;
    };
    
void quickSort(int left, int right) {
    if (left >= right) return;
    double pivot = vec[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (vec[j] <= pivot) {
            i++;
            swap(vec[i], vec[j]);
        }
    }
    swap(vec[i + 1], vec[right]);
    quickSort(left, i);
    quickSort(i + 2, right);
}

void* pthread_sort(void* arg) {
    Section* section = (Section*)arg;
    quickSort(section->left, section->right);
    delete section;
    pthread_barrier_wait(&barrier);
    return nullptr;
}

void merge(int current_size) {
    vector<int> index(THREADS_NUM), index_most(THREADS_NUM);
    long sortNumPerThread = current_size / THREADS_NUM;
    
    for (int i = 0; i < THREADS_NUM; ++i) {
        index[i] = i * sortNumPerThread;
        index_most[i] = (i == THREADS_NUM - 1) ? current_size : (i + 1) * sortNumPerThread;
    }
    
    for (int i = 0; i < current_size; ++i) {
        double min_num = DBL_MAX;
        int min_index = 0;
        for (int j = 0; j < THREADS_NUM; ++j) {
            if (index[j] < index_most[j] && vec[index[j]] < min_num) {
                min_num = vec[index[j]];
                min_index = j;
            }
        }
        sortedVec[i] = min_num;
        index[min_index]++;
    }
}

int main() {
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    DWORD_PTR mask = 0;
    for (int i = 0; i < CPU_CORES; i++) {
        mask |= (1 << i);
    }
    SetProcessAffinityMask(GetCurrentProcess(), mask);

    int sizes[] = {100, 1000, 10000, 100000, 1000000, 10000000, 100000000};
    ofstream perfFile("d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\parallel_performance_" 
        + to_string(THREADS_NUM) + "threads_" + to_string(CPU_CORES) + "cores.txt");
        perfFile << "使用线程数: " << THREADS_NUM << "\n"
        << "使用CPU核心数: " << CPU_CORES << "\n"
        << "并行阈值: 10000\n\n";

    for (int size : sizes) {
        string inputFile = "d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\test\\input_" 
                          + to_string(size) + ".txt";
        ifstream inFile(inputFile);
        
        vec.clear();
        double num;
        while (inFile >> num) {
            vec.push_back(num);
        }
        inFile.close();
        
        int current_size = vec.size();
        sortedVec.resize(current_size);
        long sortNumPerThread = current_size / THREADS_NUM;

        pthread_t tid[THREADS_NUM];
        pthread_barrier_init(&barrier, NULL, THREADS_NUM + 1);
        
        auto start = system_clock::now();
        for (int i = 0; i < THREADS_NUM; ++i) {
            Section* section = new Section();
            section->left = i * sortNumPerThread;
            section->right = (i == THREADS_NUM - 1) ? current_size - 1 : (i + 1) * sortNumPerThread - 1;
            pthread_create(&tid[i], NULL, pthread_sort, section);
        }

        pthread_barrier_wait(&barrier);
        merge(current_size);
        
        auto end = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        perfFile << "数据规模: " << size << ", 耗时: " 
                << duration.count() * 1e-6 << " 秒" << endl;

        for (int i = 0; i < THREADS_NUM; ++i) {
            pthread_join(tid[i], NULL);
        }
        pthread_barrier_destroy(&barrier);
    }
    perfFile.close();
    return 0;
}