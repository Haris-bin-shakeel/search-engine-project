#pragma once

/**
 * Industry-Level Memory Monitoring Utility
 * 
 * Standalone utility for observing memory usage during search engine operations.
 * This is COMPLETELY OPTIONAL and does NOT affect runtime behavior.
 * 
 * Usage:
 *   #ifdef ENABLE_MEMORY_MONITORING
 *   MemoryMonitor::log_snapshot("After Semantic Engine Load");
 *   #endif
 */

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <string>
#include <iostream>
#include <iomanip>

class MemoryMonitor {
public:
    /**
     * Log current memory usage with a label
     * Returns true if successful, false on error
     */
    static bool log_snapshot(const std::string& label);
    
    /**
     * Get current working set size in MB
     */
    static double get_working_set_mb();
    
    /**
     * Get peak working set size in MB
     */
    static double get_peak_working_set_mb();
    
    /**
     * Get private memory usage in MB
     */
    static double get_private_memory_mb();
    
    /**
     * Print detailed memory report
     */
    static void print_report(const std::string& label);

private:
    static bool get_memory_info(PROCESS_MEMORY_COUNTERS_EX& pmc);
};

#else
// Non-Windows stub (for portability)
class MemoryMonitor {
public:
    static bool log_snapshot(const std::string& label) { return false; }
    static double get_working_set_mb() { return 0.0; }
    static double get_peak_working_set_mb() { return 0.0; }
    static double get_private_memory_mb() { return 0.0; }
    static void print_report(const std::string& label) {}
};
#endif
