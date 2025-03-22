#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <pthread.h>
#include <queue>
#include <mutex>
using namespace std;

const int NUM_THREADS = 15;  // 线程数量
const int THRESHOLD = 10000; // 并行阈值

struct SortTask {
    vector<int>* arr;
    int low;
    int high;
    SortTask(vector<int>* a, int l, int h) : arr(a), low(l), high(h) {}
};

queue<SortTask> taskQueue;
mutex queueMutex;
pthread_t threads[NUM_THREADS];
bool shouldTerminate = false;

int partition(vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

void serialQuickSort(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        serialQuickSort(arr, low, pi - 1);
        serialQuickSort(arr, pi + 1, high);
    }
}

void addTask(SortTask task) {
    lock_guard<mutex> lock(queueMutex);
    taskQueue.push(task);
}

void* workerThread(void* arg) {
    while (true) {
        SortTask task(nullptr, 0, 0);
        {
            lock_guard<mutex> lock(queueMutex);
            if (shouldTerminate && taskQueue.empty()) {
                break;
            }
            if (!taskQueue.empty()) {
                task = taskQueue.front();
                taskQueue.pop();
            } else {
                continue;
            }
        }

        if (task.high - task.low <= THRESHOLD) {
            serialQuickSort(*task.arr, task.low, task.high);
        } else {
            int pi = partition(*task.arr, task.low, task.high);
            if (pi - 1 > task.low) {
                addTask(SortTask(task.arr, task.low, pi - 1));
            }
            if (task.high > pi + 1) {
                addTask(SortTask(task.arr, pi + 1, task.high));
            }
        }
    }
    return nullptr;
}

void parallelQuickSort(vector<int>& arr) {
    shouldTerminate = false;
    
    // 创建工作线程
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], nullptr, workerThread, nullptr);
    }
    
    // 添加初始任务
    addTask(SortTask(&arr, 0, arr.size() - 1));
    
    // 等待所有任务完成
    shouldTerminate = true;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }
}

int main() {
    // 定义测试文件大小数组
    int sizes[] = {100, 1000, 10000, 100000, 1000000};
    
    // 创建性能记录文件
    string performanceFileName = "d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\parallel_performance_"
                               + to_string(NUM_THREADS) + "threads.txt";
    ofstream performanceFile(performanceFileName);
    if (!performanceFile) {
        cout << "无法创建性能记录文件！" << endl;
        return 1;
    }
    
    // 记录线程信息
    performanceFile << "使用线程数: " << NUM_THREADS << endl;
    performanceFile << "并行阈值: " << THRESHOLD << endl << endl;
    
    // 对每个测试文件进行排序
    for (int size : sizes) {
        vector<int> arr;
        string inputFile = "d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\test\\input_" 
                          + to_string(size) + ".txt";
        string outputFile = "d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\test\\parallel_" 
                           + to_string(NUM_THREADS) + "threads_output_" + to_string(size) + ".txt";
        
        // 读取输入文件
        ifstream inFile(inputFile);
        if (!inFile) {
            cout << "无法打开文件：" << inputFile << endl;
            continue;
        }
        
        int num;
        while (inFile >> num) {
            arr.push_back(num);
        }
        inFile.close();
        
        cout << "正在处理规模为 " << size << " 的数据..." << endl;
        
        // 排序并计时
        auto start = chrono::high_resolution_clock::now();
        parallelQuickSort(arr);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;
        
        // 记录性能数据
        performanceFile << "数据规模: " << size << ", 耗时: " << duration.count() << " 秒" << endl;
        
        // 保存排序结果
        ofstream outFile(outputFile);
        if (outFile) {
            for (int num : arr) {
                outFile << num << " ";
            }
            outFile.close();
        }
        
        cout << "规模 " << size << " 完成，用时: " << duration.count() << " 秒" << endl;
        
        // 清空任务队列
        while (!taskQueue.empty()) {
            taskQueue.pop();
        }
    }
    
    performanceFile.close();
    cout << "所有测试完成，性能数据已保存到 " << performanceFileName << endl;
    return 0;
}