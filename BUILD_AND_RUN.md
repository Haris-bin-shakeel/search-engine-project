# Build and Run Instructions

## Compilation

The project has been compiled successfully! The executable `search_engine.exe` is ready to run.

### Manual Compilation Command (if you need to recompile)

```powershell
cd "C:\Users\Muhammad Haris\OneDrive\Desktop\Data_structure_Project\search engine project"
g++ -std=c++17 -I./include src/main.cpp src/stage1_lexicon.cpp src/stage2_forward_index.cpp src/stage3_inverted_index.cpp src/stage4_ranking.cpp src/stage5_query_engine.cpp src/stage6_barrels.cpp src/stage7_semantic.cpp src/stage8_autocomplete.cpp src/dynamic_indexer.cpp -o search_engine.exe -O2
```

## Running the Program

### Option 1: Run from PowerShell/Terminal

```powershell
cd "C:\Users\Muhammad Haris\OneDrive\Desktop\Data_structure_Project\search engine project"
.\search_engine.exe
```

### Option 2: Double-click the executable

Navigate to the project folder and double-click `search_engine.exe`.

## Usage Guide

Once the program starts, you'll see an interactive CLI. Here are the commands:

### 1. Normal Search Query
Just type your search query and press Enter:
```
> car
> automobile hire
> fast vehicle
```

### 2. Add New Document Dynamically
Use the `ADD:` command followed by the document text:
```
> ADD: this is a new document about artificial intelligence
> ADD: machine learning and deep learning are fascinating topics
```

### 3. Autocomplete Suggestions
Use the `AUTO:` command followed by a prefix:
```
> AUTO: car
> AUTO: auto
```

### 4. Exit the Program
Type `EXIT` or `QUIT`:
```
> EXIT
```

## What to Expect

When you first run the program:
1. It will load the corpus from `data/corpus_tokens_final_clean.txt`
2. Build all stages (1-9)
3. Load any existing delta index if present
4. Start the interactive CLI

The program will show diagnostic logs for each stage initialization, making it easy to track progress.

## File Structure

- **Executable**: `search_engine.exe` (in project root)
- **Data files**: `data/corpus_tokens_final_clean.txt` (required)
- **Delta files**: `data/delta_*.dat` (created automatically when you add documents)

Enjoy using your search engine! 🚀
