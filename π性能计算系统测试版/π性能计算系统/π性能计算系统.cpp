#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include <gmpxx.h>

// 跨平台头文件
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <pwd.h>
#endif

#if defined(USE_CUDA)
#include <cuda_runtime.h>
#include <nvml.h>
#elif defined(USE_HIP)
#include <hip/hip_runtime.h>
#include <rocm_smi/rocm_smi.h>
#endif

using namespace std;

// ================== 多语言配置 ==================
unordered_map<string, unordered_map<string, string>> i18n = {
    {"zh", {
        {"system_name", "=== π性能计算系统 ==="},
        {"select_lang", "请选择语言 (1-简体中文 2-English): "},
        {"select_precision", "请选择计算精度:\n1. 1000位\n2. 10000位\n选择: "},
        // ... 其他中文翻译
    }},
    {"en", {
        {"system_name", "=== π Performance Computing System ==="},
        {"select_lang", "Select language (1-Chinese 2-English): "},
        {"select_precision", "Select precision:\n1. 1000 digits\n2. 10000 digits\nChoice: "},
        // ... 其他英文翻译
    }}
};

// ================== 硬件检测系统 ==================
class HardwareDetector {
public:
    struct GPUInfo {
        string name;
        string vendor;
        size_t vram; // MB
    };

    struct SystemInfo {
        string cpu;
        int cores;
        size_t ram; // MB
        vector<GPUInfo> gpus;
    };

    SystemInfo detect() {
        SystemInfo info;
        detect_cpu(info);
        detect_ram(info);
        detect_gpus(info);
        return info;
    }

private:
    void detect_cpu(SystemInfo& info) {
#ifdef _WIN32
        // Windows CPU检测
        IWbemLocator* pLoc = nullptr;
        CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLoc);
        // WMI查询代码...
#else
        // Linux CPU检测
        ifstream cpuinfo("/proc/cpuinfo");
        string line;
        while (getline(cpuinfo, line)) {
            if (line.find("model name") != string::npos) {
                info.cpu = line.substr(line.find(":") + 2);
                break;
            }
        }
        info.cores = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    }

    void detect_ram(SystemInfo& info) {
#ifdef _WIN32
        MEMORYSTATUSEX mem;
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);
        info.ram = mem.ullTotalPhys / (1024 * 1024);
#else
        struct sysinfo si;
        sysinfo(&si);
        info.ram = (si.totalram * si.mem_unit) / (1024 * 1024);
#endif
       
        void detect_gpus(SystemInfo & info); {
#if defined(USE_CUDA)
        // NVIDIA GPU检测
        int count;
        cudaGetDeviceCount(&count);
        for (int i = 0; i < count; i++) {
            cudaDeviceProp prop;
            cudaGetDeviceProperties(&prop, i);
            info.gpus.push_back({
                prop.name,
                "NVIDIA",
                prop.totalGlobalMem / (1024 * 1024)
                });
        }
#elif defined(USE_HIP)
        // AMD GPU检测
        uint32_t count;
        rsmi_num_monitor_devices(&count);
        for (uint32_t i = 0; i < count; i++) {
            rsmi_device_name_get(i, buffer, 64);
            info.gpus.push_back({
                buffer,
                "AMD",
                get_amd_vram(i)
                });
        }
#endif
    }
};

// ================== π计算本体 ==================
class PiCalculator {
public:
    enum Precision { KILO = 1000, MEGA = 10000 };

    string compute(Precision precision, const HardwareDetector::SystemInfo& hw) {
        mpf_set_default_prec(precision * 4);

        if (hw.gpus.empty()) {
            return cpu_nilakantha(precision);
        }
        else {
            for (const auto& gpu : hw.gpus) {
                if (gpu.vendor == "NVIDIA") {
                    return cuda_nilakantha(precision, hw);
                }
                else if (gpu.vendor == "AMD") {
                    return hip_nilakantha(precision, hw);
                }
            }
        }
        return cpu_nilakantha(precision);
    }

private:
    string cpu_nilakantha(int digits) {
        mpf_class pi("3.0");
        mpf_class sign(1.0);
        // CPU实现...
        return pi.get_str(digits);
    }

#if defined(USE_CUDA)
    string cuda_nilakantha(int digits, const HardwareDetector::SystemInfo& hw) {
        // CUDA加速实现...
        return "3.1415926535...";
    }
#elif defined(USE_HIP)
    string hip_nilakantha(int digits, const HardwareDetector::SystemInfo& hw) {
        // HIP加速实现...
        return "3.1415926535...";
    }
#endif
};

// ================== 用户界面 ==================
class UserInterface {
public:
    struct Config {
        string lang;
        PiCalculator::Precision precision;
    };

    Config get_config() {
        Config config;
        select_language(config);
        select_precision(config);
        return config;
    }

private:
    void select_language(Config& config) {
        cout << i18n["zh"]["select_lang"];
        int choice;
        cin >> choice;
        config.lang = (choice == 1) ? "zh" : "en";
    }

    void select_precision(Config& config) {
        cout << i18n[config.lang]["select_precision"];
        int choice;
        cin >> choice;
        config.precision = (choice == 1) ?
            PiCalculator::KILO : PiCalculator::MEGA;
    }
};

// ================== 主程序 ==================
int main() {
    try {
        // 初始化系统
        UserInterface ui;
        HardwareDetector hw_detector;
        PiCalculator calculator;

        // 获取用户配置
        auto config = ui.get_config();
        auto hw_info = hw_detector.detect();

        // 执行计算
        string pi = calculator.compute(config.precision, hw_info);

        // 生成报告
        string filename = (config.lang == "zh") ?
            "π计算结果.txt" : "pi_result.txt";
        ofstream out(filename);
        out << "精确到 " << config.precision << " 位的π值:\n"
            << format_output(pi, config.precision);

        cout << (config.lang == "zh" ? "计算完成！" : "Calculation completed!")
            << endl;

    }
    catch (const exception& e) {
        cerr << "错误: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// 辅助函数：格式化输出
string format_output(const string& pi, int precision) {
    const int line_width = 100;
    string formatted;

    for (int i = 0; i < precision; i += line_width) {
        int end = min(i + line_width, precision);
        formatted += pi.substr(i, end - i) + "\n";
    }
    return formatted;
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include <gmpxx.h> // 确保此路径正确

    // 跨平台头文件
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "gmp.lib") // 链接 GMP 库
#pragma comment(lib, "gmpxx.lib") // 链接 GMP++ 库
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <pwd.h>
#endif
}