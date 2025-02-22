#include <iostream>
#include <cmath>
#include <random>
#include <chrono>

using namespace std;

// ================== 蒙特卡洛方法 ==================
double monte_carlo_pi(int samples) {
    random_device rd;
    mt19937 generator(rd());
    uniform_real_distribution<double> distribution(-1.0, 1.0);

    int inside_circle = 0;

    for (int i = 0; i < samples; ++i) {
        double x = distribution(generator);
        double y = distribution(generator);
        if (x * x + y * y <= 1.0) {
            ++inside_circle;
        }
    }

    return 4.0 * inside_circle / samples;
}

// =============== 尼拉坎萨级数方法 ===============
double nilakantha_pi(int iterations) {
    double pi = 3.0;
    double sign = 1.0;

    for (int n = 2; n < iterations * 2; n += 2) {
        double denominator = n * (n + 1) * (n + 2);
        pi += sign * (4.0 / denominator);
        sign *= -1; // 交替符号
    }

    return pi;
}

// ================ 主程序 ================
int main() {
    const double PI = 3.14159265358979323846;
    int choice;
    int precision;

    cout << "=== 圆周率计算器 ===" << endl;
    cout << "1. 蒙特卡洛方法" << endl;
    cout << "2. 尼拉坎萨级数" << endl;
    cout << "选择计算方法 (1/2): ";
    cin >> choice;

    cout << "输入计算精度（蒙特卡洛样本数 或 级数迭代次数）: ";
    cin >> precision;

    auto start = chrono::high_resolution_clock::now();
    double result = 0;

    switch (choice) {
    case 1:
        result = monte_carlo_pi(precision);
        break;
    case 2:
        result = nilakantha_pi(precision);
        break;
    default:
        cout << "无效选择！" << endl;
        return 1;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout.precision(15);
    cout << "\n计算结果: " << result << endl;
    cout << "实际PI值: " << PI << endl;
    cout << "绝对误差: " << abs(result - PI) << endl;
    cout << "计算耗时: " << duration.count() << " 秒" << endl;

    return 0;
}