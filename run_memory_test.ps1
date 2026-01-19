# Memory Test Script for Search Engine
# This script helps you test memory usage without modifying code

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   Search Engine Memory Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if executable exists
if (-not (Test-Path "search_engine.exe")) {
    Write-Host "ERROR: search_engine.exe not found!" -ForegroundColor Red
    Write-Host "Please compile the project first." -ForegroundColor Yellow
    exit 1
}

Write-Host "Step 1: Starting search engine..." -ForegroundColor Green
Write-Host ""

# Start the process
$process = Start-Process -FilePath ".\search_engine.exe" -PassThru -NoNewWindow

if (-not $process) {
    Write-Host "ERROR: Failed to start search_engine.exe" -ForegroundColor Red
    exit 1
}

Write-Host "Process started with PID: $($process.Id)" -ForegroundColor Yellow
Write-Host ""
Write-Host "Step 2: Monitoring memory usage..." -ForegroundColor Green
Write-Host "Press Ctrl+C to stop monitoring" -ForegroundColor Yellow
Write-Host ""

# Function to format bytes to MB
function Format-MemoryMB {
    param([long]$bytes)
    return [math]::Round($bytes / 1MB, 2)
}

# Monitor memory
$checkpoints = @()
$startTime = Get-Date

try {
    while ($true) {
        $proc = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
        if (-not $proc) {
            Write-Host "Process has exited." -ForegroundColor Yellow
            break
        }
        
        $ws = Format-MemoryMB $proc.WorkingSet
        $pws = Format-MemoryMB $proc.PeakWorkingSet64
        $private = Format-MemoryMB $proc.PrivateMemorySize64
        $elapsed = (Get-Date) - $startTime
        
        $timestamp = Get-Date -Format 'HH:mm:ss'
        $status = "OK"
        $color = "White"
        
        # Check limits (assuming 80k docs dataset)
        if ($ws -gt 2000) {
            $status = "EXCEEDED 2GB LIMIT!"
            $color = "Red"
        } elseif ($ws -gt 1500) {
            $status = "WARNING: Approaching limit"
            $color = "Yellow"
        }
        
        Write-Host "$timestamp | Working Set: $ws MB | Peak: $pws MB | Private: $private MB | Status: $status" -ForegroundColor $color
        
        # Record checkpoint
        $checkpoints += [PSCustomObject]@{
            Time = $timestamp
            WorkingSetMB = $ws
            PeakMB = $pws
            PrivateMB = $private
            Status = $status
        }
        
        Start-Sleep -Seconds 2
    }
} catch {
    Write-Host "`nMonitoring stopped." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   Memory Test Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($checkpoints.Count -gt 0) {
    $maxWS = ($checkpoints | Measure-Object -Property WorkingSetMB -Maximum).Maximum
    $maxPeak = ($checkpoints | Measure-Object -Property PeakMB -Maximum).Maximum
    $avgWS = ($checkpoints | Measure-Object -Property WorkingSetMB -Average).Average
    
    Write-Host "Maximum Working Set: $maxWS MB" -ForegroundColor $(if ($maxWS -gt 2000) { "Red" } else { "Green" })
    Write-Host "Peak Working Set:    $maxPeak MB" -ForegroundColor $(if ($maxPeak -gt 2000) { "Red" } else { "Green" })
    Write-Host "Average Working Set: $([math]::Round($avgWS, 2)) MB" -ForegroundColor Green
    Write-Host ""
    
    # Validation
    Write-Host "Validation Results:" -ForegroundColor Cyan
    if ($maxWS -le 2000) {
        Write-Host "  ✅ PASS: Working Set under 2GB limit" -ForegroundColor Green
    } else {
        Write-Host "  ❌ FAIL: Working Set exceeds 2GB limit" -ForegroundColor Red
    }
    
    if ($maxPeak -le 2000) {
        Write-Host "  ✅ PASS: Peak memory under 2GB limit" -ForegroundColor Green
    } else {
        Write-Host "  ❌ FAIL: Peak memory exceeds 2GB limit" -ForegroundColor Red
    }
} else {
    Write-Host "No memory data collected." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Test completed!" -ForegroundColor Green
