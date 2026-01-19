# How to Run Memory Tests - Step by Step Guide

## 🚀 Quick Start (Easiest Method)

### Option 1: Use the Automated Script

1. **Open PowerShell in the project directory:**
   ```powershell
   cd "C:\Users\Muhammad Haris\OneDrive\Desktop\Data_structure_Project\search engine project"
   ```

2. **Run the memory test script:**
   ```powershell
   .\run_memory_test.ps1
   ```

3. **The script will:**
   - Start the search engine
   - Monitor memory every 2 seconds
   - Show real-time memory usage
   - Generate a summary report
   - Validate against 2GB limit

**Press Ctrl+C to stop monitoring when done.**

---

## 📊 Method 2: Manual Testing with Task Manager

### Step-by-Step:

1. **Start the search engine:**
   ```powershell
   .\search_engine.exe
   ```

2. **Open Task Manager:**
   - Press `Ctrl+Shift+Esc`
   - Click "More details" if needed
   - Go to "Details" tab

3. **Add Memory Column:**
   - Right-click on column headers
   - Select "Select columns"
   - Check "Memory (private working set)"
   - Click OK

4. **Find the process:**
   - Look for `search_engine.exe`
   - Watch the "Memory (private working set)" column

5. **Monitor at key points:**
   - **After startup:** Should be ~500-800 MB
   - **After initialization:** Should be ~800-1000 MB
   - **After semantic build:** Should be ~900-1200 MB
   - **During queries:** Should stay stable
   - **After adding docs:** Should increase slightly
   - **After compaction:** Should decrease

6. **Check Peak Memory:**
   - Right-click → "Select columns" → Check "Peak working set"
   - This shows the maximum memory used

---

## 💻 Method 3: PowerShell One-Liner

### Get Current Memory:
```powershell
Get-Process search_engine -ErrorAction SilentlyContinue | 
    Select-Object @{Name="WorkingSet(MB)";Expression={[math]::Round($_.WorkingSet/1MB,2)}}, 
                  @{Name="Peak(MB)";Expression={[math]::Round($_.PeakWorkingSet64/1MB,2)}}
```

### Continuous Monitoring:
```powershell
while ($true) {
    $proc = Get-Process search_engine -ErrorAction SilentlyContinue
    if ($proc) {
        $ws = [math]::Round($proc.WorkingSet / 1MB, 2)
        $pws = [math]::Round($proc.PeakWorkingSet64 / 1MB, 2)
        Write-Host "$(Get-Date -Format 'HH:mm:ss') | Working Set: $ws MB | Peak: $pws MB"
    }
    Start-Sleep -Seconds 2
}
```

---

## 🔧 Method 4: With Memory Monitor Utility (Advanced)

### Step 1: Compile with Memory Monitoring

```powershell
g++ -std=c++17 -I./include -DENABLE_MEMORY_MONITORING `
    src/main.cpp src/stage1_lexicon.cpp src/stage2_forward_index.cpp `
    src/stage3_inverted_index.cpp src/stage4_ranking.cpp `
    src/stage5_query_engine.cpp src/stage6_barrels.cpp `
    src/stage7_semantic.cpp src/stage8_autocomplete.cpp `
    src/dynamic_indexer.cpp src/memory_monitor.cpp `
    -o search_engine.exe -O2 -lpsapi
```

### Step 2: Add Snapshot Hooks (Optional)

Add to `src/stage7_semantic.cpp`:

```cpp
#ifdef ENABLE_MEMORY_MONITORING
#include "memory_monitor.h"
#endif

void SemanticEngine::build_document_vectors(...) {
    // ... existing code ...
    
#ifdef ENABLE_MEMORY_MONITORING
    MemoryMonitor::print_report("After Semantic Vector Build");
#endif
}
```

### Step 3: Run and Observe

The program will automatically print memory reports.

---

## ✅ Validation Checklist

### For 80k Documents:

- [ ] **Initialization:** < 1 GB ✅
- [ ] **After Lexicon:** < 1.1 GB ✅
- [ ] **After Forward Index:** < 1.2 GB ✅
- [ ] **After Inverted Index:** < 1.3 GB ✅
- [ ] **After Semantic Build:** < 1.5 GB ✅
- [ ] **After 100 Dynamic Docs:** < 1.6 GB ✅
- [ ] **After Compaction:** < 1.5 GB ✅
- [ ] **Peak Memory:** < 2 GB ✅

### Expected Memory Profile:

```
Time    | Memory  | Event
--------|---------|------------------
00:00   | 50 MB   | Program start
00:05   | 150 MB  | Corpus loaded
00:10   | 300 MB  | Lexicon built
00:15   | 600 MB  | Forward index built
00:20   | 850 MB  | Inverted index built
00:25   | 900 MB  | Semantic vectors built
00:30   | 920 MB  | Ready for queries
```

---

## 🎯 Quick Test Procedure

### Test 1: Basic Memory Check

1. **Start the program:**
   ```powershell
   .\search_engine.exe
   ```

2. **Wait for initialization** (watch the console output)

3. **Check memory in Task Manager:**
   - Should be under 1 GB after initialization
   - Should be under 1.5 GB after semantic build

4. **Run a few queries:**
   ```
   > car
   > server
   > friend
   ```

5. **Check memory again:**
   - Should stay stable (no leaks)

### Test 2: Dynamic Indexing Memory

1. **Add documents:**
   ```
   > ADD: this is a test document about memory
   > ADD: another document for testing
   > ADD: third document to check memory growth
   ```

2. **Monitor memory:**
   - Should increase slightly (~10-20 MB per document)
   - Should stay under 2 GB

3. **Run compaction:**
   ```
   > COMPACT
   ```

4. **Check memory:**
   - Should decrease after compaction

### Test 3: Peak Memory Test

1. **Add many documents:**
   ```
   > ADD: document one
   > ADD: document two
   ... (add 50-100 documents)
   ```

2. **Monitor peak memory:**
   - Should stay under 2 GB for 80k base + 100 dynamic docs

3. **Run compaction:**
   ```
   > COMPACT
   ```

4. **Verify memory decreases**

---

## 📈 Understanding the Results

### Good Signs ✅
- Memory stays under 1 GB during normal operation
- Memory increases linearly with documents
- Memory decreases after compaction
- No unbounded growth

### Warning Signs ⚠️
- Memory exceeds 1.5 GB for 80k docs
- Memory keeps growing without compaction
- Memory doesn't decrease after compaction

### Critical Issues ❌
- Memory exceeds 2 GB for 80k docs
- Memory grows unbounded
- Memory leaks (keeps growing during queries)

---

## 🛠️ Troubleshooting

### If Memory is Too High:

1. **Check dataset size:**
   ```powershell
   (Get-Content .\data\corpus_tokens_final_clean.txt | Measure-Object -Line).Lines
   ```

2. **Check delta index:**
   ```powershell
   Get-ChildItem .\data\delta_*.dat | Measure-Object -Property Length -Sum
   ```

3. **Run compaction:**
   ```
   > COMPACT
   ```

4. **Restart program** to clear any issues

### If Script Doesn't Work:

1. **Check PowerShell execution policy:**
   ```powershell
   Get-ExecutionPolicy
   ```

2. **If restricted, allow scripts:**
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   ```

---

## 📝 Expected Output Examples

### From Automated Script:
```
========================================
   Search Engine Memory Test
========================================

Step 1: Starting search engine...
Process started with PID: 12345

Step 2: Monitoring memory usage...
Press Ctrl+C to stop monitoring

14:30:15 | Working Set: 850.23 MB | Peak: 920.45 MB | Private: 780.12 MB | Status: OK
14:30:17 | Working Set: 855.67 MB | Peak: 920.45 MB | Private: 785.34 MB | Status: OK
...

========================================
   Memory Test Summary
========================================

Maximum Working Set: 920.45 MB
Peak Working Set:    920.45 MB
Average Working Set: 875.23 MB

Validation Results:
  ✅ PASS: Working Set under 2GB limit
  ✅ PASS: Peak memory under 2GB limit
```

### From Task Manager:
- **Memory (private working set):** ~850 MB
- **Peak working set:** ~920 MB

---

## 🎓 Next Steps

1. **Run the automated script** to get baseline measurements
2. **Compare with expected values** from the validation report
3. **Test with different dataset sizes** if available
4. **Monitor during different operations** (queries, indexing, compaction)

---

## 📚 Additional Resources

- **MEMORY_TESTING_GUIDE.md** - Comprehensive guide
- **MEMORY_VALIDATION_REPORT.md** - Architecture analysis
- **QUICK_MEMORY_TEST.md** - Quick reference

---

**Ready to test? Run:**
```powershell
.\run_memory_test.ps1
```
