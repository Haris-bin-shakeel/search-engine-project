# 🚀 Quick Start: Run Memory Tests NOW

## ✅ Everything is Ready!

Your system is set up:
- ✅ `search_engine.exe` exists
- ✅ PowerShell scripts can run
- ✅ Memory test script created

---

## 🎯 EASIEST WAY: Run the Automated Script

### Step 1: Open PowerShell
Press `Win + X` → Select "Windows PowerShell" or "Terminal"

### Step 2: Navigate to Project
```powershell
cd "C:\Users\Muhammad Haris\OneDrive\Desktop\Data_structure_Project\search engine project"
```

### Step 3: Run the Test
```powershell
.\run_memory_test.ps1
```

**That's it!** The script will:
- Start your search engine
- Monitor memory every 2 seconds
- Show real-time updates
- Generate a summary report
- Validate against 2GB limit

**Press `Ctrl+C` to stop when done.**

---

## 📊 What You'll See

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
14:30:19 | Working Set: 860.12 MB | Peak: 920.45 MB | Private: 790.56 MB | Status: OK
...

[Press Ctrl+C]

========================================
   Memory Test Summary
========================================

Maximum Working Set: 920.45 MB
Peak Working Set:    920.45 MB
Average Working Set: 875.23 MB

Validation Results:
  ✅ PASS: Working Set under 2GB limit
  ✅ PASS: Peak memory under 2GB limit

Test completed!
```

---

## 🔍 Alternative: Manual Testing with Task Manager

### Step 1: Start the Program
```powershell
.\search_engine.exe
```

### Step 2: Open Task Manager
- Press `Ctrl+Shift+Esc`
- Go to "Details" tab
- Find `search_engine.exe`

### Step 3: Add Memory Column
- Right-click column headers → "Select columns"
- Check "Memory (private working set)"
- Check "Peak working set"
- Click OK

### Step 4: Watch Memory
- **After startup:** Should be ~500-800 MB
- **After initialization:** Should be ~800-1000 MB
- **After semantic build:** Should be ~900-1200 MB

---

## ⚡ Quick PowerShell Commands

### Get Current Memory (One-Liner):
```powershell
Get-Process search_engine -ErrorAction SilentlyContinue | Select-Object @{Name="Memory(MB)";Expression={[math]::Round($_.WorkingSet/1MB,2)}}, @{Name="Peak(MB)";Expression={[math]::Round($_.PeakWorkingSet64/1MB,2)}}
```

### Continuous Monitoring:
```powershell
while ($true) {
    $proc = Get-Process search_engine -ErrorAction SilentlyContinue
    if ($proc) {
        $ws = [math]::Round($proc.WorkingSet / 1MB, 2)
        Write-Host "$(Get-Date -Format 'HH:mm:ss') | Memory: $ws MB"
    }
    Start-Sleep -Seconds 2
}
```

---

## ✅ What to Look For

### Good Signs ✅
- Memory stays under 1 GB during normal operation
- Memory increases smoothly (no sudden spikes)
- Memory decreases after compaction
- Peak memory under 2 GB for 80k docs

### Expected Values:
- **80k documents:** ~850 MB (under 2GB ✅)
- **120k documents:** ~1.2 GB (under 4GB ✅)

---

## 🎓 Test Scenarios

### Test 1: Basic Memory Check
1. Run `.\search_engine.exe`
2. Wait for initialization
3. Check Task Manager → Should be < 1 GB

### Test 2: Dynamic Indexing
1. Add documents: `ADD: test document one`
2. Monitor memory → Should increase slightly
3. Run `COMPACT` → Memory should decrease

### Test 3: Peak Memory
1. Add 50-100 documents
2. Check peak memory → Should stay < 2 GB
3. Run compaction → Should decrease

---

## 🆘 Troubleshooting

### Script Won't Run?
```powershell
# Allow scripts to run
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Can't Find Process?
```powershell
# List all processes
Get-Process | Where-Object {$_.ProcessName -like "*search*"}
```

### Memory Too High?
1. Check dataset size
2. Run `COMPACT` command
3. Restart program

---

## 📝 Next Steps

1. **Run the automated script** → `.\run_memory_test.ps1`
2. **Compare results** with expected values
3. **Test different scenarios** (queries, indexing, compaction)
4. **Review the summary** at the end

---

**Ready? Run this command:**
```powershell
.\run_memory_test.ps1
```
