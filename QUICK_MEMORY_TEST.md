# Quick Memory Test - CLI Procedures

## Method 1: Windows Task Manager (Easiest)

1. **Start the program:**
   ```powershell
   .\search_engine.exe
   ```

2. **Open Task Manager:**
   - Press `Ctrl+Shift+Esc`
   - Go to "Details" tab
   - Find `search_engine.exe`
   - Right-click → "Select columns" → Check "Memory (private working set)"

3. **Monitor at checkpoints:**
   - **After initialization:** Should show ~500-800 MB
   - **After semantic build:** Should show ~800-1200 MB
   - **Peak during operation:** Should stay under 1.5 GB (for 80k docs)

---

## Method 2: PowerShell One-Liner

```powershell
# Get current memory usage
Get-Process search_engine -ErrorAction SilentlyContinue | 
    Select-Object @{Name="WorkingSet(MB)";Expression={[math]::Round($_.WorkingSet/1MB,2)}}, 
                  @{Name="Peak(MB)";Expression={[math]::Round($_.PeakWorkingSet64/1MB,2)}}
```

**Expected Output:**
```
WorkingSet(MB) Peak(MB)
-------------- --------
        850.23   920.45
```

---

## Method 3: Continuous Monitoring Script

Save as `monitor_memory.ps1`:

```powershell
$processName = "search_engine"
Write-Host "Monitoring $processName memory usage..." -ForegroundColor Green
Write-Host "Press Ctrl+C to stop`n" -ForegroundColor Yellow

while ($true) {
    $proc = Get-Process $processName -ErrorAction SilentlyContinue
    if ($proc) {
        $ws = [math]::Round($proc.WorkingSet / 1MB, 2)
        $pws = [math]::Round($proc.PeakWorkingSet64 / 1MB, 2)
        $timestamp = Get-Date -Format 'HH:mm:ss'
        Write-Host "$timestamp | Working Set: $ws MB | Peak: $pws MB"
        
        # Alert if over limit
        if ($ws -gt 2000) {
            Write-Host "  ⚠️  WARNING: Exceeding 2GB limit!" -ForegroundColor Red
        }
    } else {
        Write-Host "$(Get-Date -Format 'HH:mm:ss') | Process not running"
    }
    Start-Sleep -Seconds 2
}
```

**Usage:**
```powershell
# Terminal 1: Run the program
.\search_engine.exe

# Terminal 2: Run monitoring
.\monitor_memory.ps1
```

---

## Method 4: Memory Monitor Utility (If Compiled)

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

Add to `src/stage7_semantic.cpp` (example):

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

The program will automatically print memory reports at each snapshot.

---

## Validation Checklist

### For 80k Documents Dataset

- [ ] **Initialization:** < 1 GB ✅
- [ ] **After Lexicon:** < 1.1 GB ✅
- [ ] **After Forward Index:** < 1.2 GB ✅
- [ ] **After Inverted Index:** < 1.3 GB ✅
- [ ] **After Semantic Build:** < 1.5 GB ✅
- [ ] **After 100 Dynamic Docs:** < 1.6 GB ✅
- [ ] **After Compaction:** < 1.5 GB ✅
- [ ] **Peak Memory:** < 2 GB ✅

### For 120k Documents Dataset

- [ ] **Initialization:** < 1.5 GB ✅
- [ ] **After Semantic Build:** < 2.5 GB ✅
- [ ] **After 200 Dynamic Docs:** < 2.8 GB ✅
- [ ] **After Compaction:** < 2.5 GB ✅
- [ ] **Peak Memory:** < 3 GB ✅

---

## Expected Memory Profile

### Normal Operation (80k docs)
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
00:35   | 950 MB  | After 10 queries
00:40   | 980 MB  | After adding 5 docs
00:45   | 960 MB  | After compaction
```

### Peak Memory Events
- **Semantic vector build:** Highest memory spike
- **Delta compaction:** Temporary increase during merge
- **Large query processing:** Minimal increase

---

## Troubleshooting

### If Memory Exceeds Limits

1. **Check dataset size:**
   ```powershell
   (Get-Content .\data\corpus_tokens_final_clean.txt | Measure-Object -Line).Lines
   ```

2. **Check delta index size:**
   ```powershell
   Get-ChildItem .\data\delta_*.dat | Measure-Object -Property Length -Sum
   ```

3. **Run compaction:**
   ```
   > COMPACT
   ```

4. **Restart program** to clear any memory leaks

### Common Issues

- **Memory grows unbounded:** Likely delta index not compacted
- **High peak during semantic build:** Normal, should stabilize
- **Memory doesn't decrease after compaction:** May need restart

---

## Quick Reference

| Command | Purpose |
|---------|---------|
| `Get-Process search_engine \| Select-Object WorkingSet` | Current memory |
| `Get-Process search_engine \| Select-Object PeakWorkingSet64` | Peak memory |
| Task Manager → Details → Memory column | Visual monitoring |

**Memory Limits:**
- 80k docs: ≤ 2 GB
- 120k docs: ≤ 4 GB

**Alert Thresholds:**
- 80k docs: Warn at 1.5 GB
- 120k docs: Warn at 3 GB
