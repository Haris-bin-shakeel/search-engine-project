#include "memory_monitor.h"

#ifdef _WIN32

bool MemoryMonitor::get_memory_info(PROCESS_MEMORY_COUNTERS_EX& pmc) {
    HANDLE hProcess = GetCurrentProcess();
    if (hProcess == NULL) {
        return false;
    }
    
    if (!GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        CloseHandle(hProcess);
        return false;
    }
    
    CloseHandle(hProcess);
    return true;
}

bool MemoryMonitor::log_snapshot(const std::string& label) {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!get_memory_info(pmc)) {
        std::cerr << "[MEMORY] Failed to get memory info for: " << label << std::endl;
        return false;
    }
    
    double working_set_mb = pmc.WorkingSetSize / (1024.0 * 1024.0);
    double peak_mb = pmc.PeakWorkingSetSize / (1024.0 * 1024.0);
    double private_mb = pmc.PrivateUsage / (1024.0 * 1024.0);
    
    std::cout << "[MEMORY] " << label << ":" << std::endl;
    std::cout << "  Working Set: " << std::fixed << std::setprecision(2) 
              << working_set_mb << " MB" << std::endl;
    std::cout << "  Peak Working Set: " << peak_mb << " MB" << std::endl;
    std::cout << "  Private Memory: " << private_mb << " MB" << std::endl;
    
    return true;
}

double MemoryMonitor::get_working_set_mb() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!get_memory_info(pmc)) {
        return 0.0;
    }
    return pmc.WorkingSetSize / (1024.0 * 1024.0);
}

double MemoryMonitor::get_peak_working_set_mb() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!get_memory_info(pmc)) {
        return 0.0;
    }
    return pmc.PeakWorkingSetSize / (1024.0 * 1024.0);
}

double MemoryMonitor::get_private_memory_mb() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!get_memory_info(pmc)) {
        return 0.0;
    }
    return pmc.PrivateUsage / (1024.0 * 1024.0);
}

void MemoryMonitor::print_report(const std::string& label) {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!get_memory_info(pmc)) {
        std::cerr << "[MEMORY] Failed to get memory info" << std::endl;
        return;
    }
    
    double working_set_mb = pmc.WorkingSetSize / (1024.0 * 1024.0);
    double peak_mb = pmc.PeakWorkingSetSize / (1024.0 * 1024.0);
    double private_mb = pmc.PrivateUsage / (1024.0 * 1024.0);
    double page_faults = pmc.PageFaultCount;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  MEMORY REPORT: " << label << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Working Set (RAM):      " << std::setw(10) << working_set_mb << " MB" << std::endl;
    std::cout << "Peak Working Set:        " << std::setw(10) << peak_mb << " MB" << std::endl;
    std::cout << "Private Memory:          " << std::setw(10) << private_mb << " MB" << std::endl;
    std::cout << "Page Faults:             " << std::setw(10) << page_faults << std::endl;
    std::cout << "========================================\n" << std::endl;
}

#endif // _WIN32
