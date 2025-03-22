#include <iostream>
#include <fstream>
#include <random>
#include <string>
using namespace std;

void generateTestFile(int size, const string& filename) {
    ofstream outFile(filename);
    if (!outFile) {
        cout << "无法创建文件：" << filename << endl;
        return;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 1000000); 

    for (int i = 0; i < size; i++) {
        outFile << dis(gen) << " ";
    }

    outFile.close();
    cout << "已生成包含 " << size << " 个数的文件：" << filename << endl;
}

int main() {
    int sizes[] = {100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
    
    for (int size : sizes) {
        string filename = "d:\\kindoflife\\study\\Parallel-Algorithm-Analysis-and-Design\\LAB 1\\input_" 
                         + to_string(size) + ".txt";
        generateTestFile(size, filename);
    }

    cout << "所有测试文件生成完成！" << endl;
    return 0;
}