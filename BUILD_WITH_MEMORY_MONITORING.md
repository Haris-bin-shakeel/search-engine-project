# Building with Memory Monitoring (Optional)

## Overview

Memory monitoring is **completely optional** and does NOT affect production builds. It's only for testing and validation.

## Normal Build (No Memory Monitoring)

```powershell
g++ -std=c++17 -I./include src/main.cpp src/stage1_lexicon.cpp src/stage2_forward_index.cpp src/stage3_inverted_index.cpp src/stage4_ranking.cpp src/stage5_query_engine.cpp src/stage6_barrels.cpp src/stage7_semantic.cpp src/stage8_autocomplete.cpp src/dynamic_indexer.cpp -o search_engine.exe -O2
```

**Result:** Production executable with zero memory monitoring overhead.

## Build with Memory Monitoring (Testing Only)

```powershell
g++ -std=c++17 -I./include -DENABLE_MEMORY_MONITORING src/main.cpp src/stage1_lexicon.cpp src/stage2_forward_index.cpp src/stage3_inverted_index.cpp src/stage4_ranking.cpp src/stage5_query_engine.cpp src/stage6_barrels.cpp src/stage7_semantic.cpp src/stage8_autocomplete.cpp src/dynamic_indexer.cpp src/memory_monitor.cpp -o search_engine.exe -O2 -lpsapi
```

**Note:** Requires linking `-lpsapi` for Windows memory APIs.

## Adding Memory Snapshots (Optional)

To add memory snapshots to any stage file (NOT main.cpp):

```cpp
// In src/stage7_semantic.cpp (example)
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

**Important:**
- Only compiled when `-DENABLE_MEMORY_MONITORING` is set
- Zero impact on production builds
- Can be added to any stage file except main.cpp

## Usage

Once compiled with memory monitoring:

```cpp
// Anywhere in code (except main.cpp)
#ifdef ENABLE_MEMORY_MONITORING
MemoryMonitor::log_snapshot("Checkpoint Name");
MemoryMonitor::print_report("Detailed Report");
#endif
```

## Windows Requirements

Memory monitoring requires:
- Windows API (`windows.h`, `psapi.h`)
- `psapi` library (`-lpsapi` linker flag)
- Only works on Windows builds

For non-Windows platforms, the memory monitor provides stub implementations.
