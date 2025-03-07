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

// ��ƽ̨ͷ�ļ�
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

// ================== ���������� ==================
unordered_map<string, unordered_map<string, string>> i18n = {
    {"zh", {
        {"system_name", "=== �����ܼ���ϵͳ ==="},
        {"select_lang", "��ѡ������ (1-�������� 2-English): "},
        {"select_precision", "��ѡ����㾫��:\n1. 1000λ\n2. 10000λ\nѡ��: "},
        // ... �������ķ���
    }},
    {"en", {
        {"system_name", "=== �� Performance Computing System ==="},
        {"select_lang", "Select language (1-Chinese 2-English): "},
        {"select_precision", "Select precision:\n1. 1000 digits\n2. 10000 digits\nChoice: "},
        // ... ����Ӣ�ķ���
    }}
};

// ================== Ӳ�����ϵͳ ==================
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
        // Windows CPU���
        IWbemLocator* pLoc = nullptr;
        CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLoc);
        // WMI��ѯ����...
#else
        // Linux CPU���
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
        // NVIDIA GPU���
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
        // AMD GPU���
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

// ================== �м��㱾�� ==================
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
        // CPUʵ��...
        return pi.get_str(digits);
    }

#if defined(USE_CUDA)
    string cuda_nilakantha(int digits, const HardwareDetector::SystemInfo& hw) {
        // CUDA����ʵ��...
        return "3.1415926535...";
    }
#elif defined(USE_HIP)
    string hip_nilakantha(int digits, const HardwareDetector::SystemInfo& hw) {
        // HIP����ʵ��...
        return "3.1415926535...";
    }
#endif
};

// ================== �û����� ==================
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

// ================== ������ ==================
int main() {
    try {
        // ��ʼ��ϵͳ
        UserInterface ui;
        HardwareDetector hw_detector;
        PiCalculator calculator;

        // ��ȡ�û�����
        auto config = ui.get_config();
        auto hw_info = hw_detector.detect();

        // ִ�м���
        string pi = calculator.compute(config.precision, hw_info);

        // ���ɱ���
        string filename = (config.lang == "zh") ?
            "�м�����.txt" : "pi_result.txt";
        ofstream out(filename);
        out << "��ȷ�� " << config.precision << " λ�Ħ�ֵ:\n"
            << format_output(pi, config.precision);

        cout << (config.lang == "zh" ? "������ɣ�" : "Calculation completed!")
            << endl;

    }
    catch (const exception& e) {
        cerr << "����: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// ������������ʽ�����
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
#include <gmpxx.h> // ȷ����·����ȷ

    // ��ƽ̨ͷ�ļ�
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "gmp.lib") // ���� GMP ��
#pragma comment(lib, "gmpxx.lib") // ���� GMP++ ��
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <pwd.h>
#endif
}