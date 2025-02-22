#include <iostream>
#include <cmath>
#include <random>
#include <chrono>

using namespace std;

// ================== ���ؿ��巽�� ==================
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

// =============== ���������������� ===============
double nilakantha_pi(int iterations) {
    double pi = 3.0;
    double sign = 1.0;

    for (int n = 2; n < iterations * 2; n += 2) {
        double denominator = n * (n + 1) * (n + 2);
        pi += sign * (4.0 / denominator);
        sign *= -1; // �������
    }

    return pi;
}

// ================ ������ ================
int main() {
    const double PI = 3.14159265358979323846;
    int choice;
    int precision;

    cout << "=== Բ���ʼ����� ===" << endl;
    cout << "1. ���ؿ��巽��" << endl;
    cout << "2. ������������" << endl;
    cout << "ѡ����㷽�� (1/2): ";
    cin >> choice;

    cout << "������㾫�ȣ����ؿ��������� �� ��������������: ";
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
        cout << "��Чѡ��" << endl;
        return 1;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout.precision(15);
    cout << "\n������: " << result << endl;
    cout << "ʵ��PIֵ: " << PI << endl;
    cout << "�������: " << abs(result - PI) << endl;
    cout << "�����ʱ: " << duration.count() << " ��" << endl;

    return 0;
}