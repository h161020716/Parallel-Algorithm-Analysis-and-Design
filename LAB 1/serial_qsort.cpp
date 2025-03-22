#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
using namespace std;

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

void quickSort(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main() {
    // 定义测试文件大小数组
    int sizes[] = {100, 1000, 10000, 100000, 1000000};
    
    // 创建性能记录文件
    ofstream performanceFile("d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\serial_performance.txt");
    if (!performanceFile) {
        cout << "无法创建性能记录文件！" << endl;
        return 1;
    }
    
    // 对每个测试文件进行排序
    for (int size : sizes) {
        vector<int> arr;
        string inputFile = "d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\test\\input_" 
                          + to_string(size) + ".txt";
        string outputFile = "d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\test\\serial_output_" 
                           + to_string(size) + ".txt";
        
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
        quickSort(arr, 0, arr.size() - 1);
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
    }
    
    performanceFile.close();
    cout << "所有测试完成，性能数据已保存到 serial_performance.txt" << endl;
    return 0;
}