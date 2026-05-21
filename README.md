# OS-Assignment: Operating System Simulator (ossim_caitoa)

## Overall View of the Project
This project is an Operating System simulator developed as an assignment for an Operating System course. It simulates the core functionalities of an operating system, providing a hands-on approach to understanding how the OS manages hardware resources. The simulator focuses on several major components:
- **CPU Scheduling**: Implements Multi-level Queue (MLQ) scheduling to manage how processes are allocated CPU time.
- **Memory Management**: Simulates virtual and physical memory management, including single-level and multi-level paging (both 32-bit and 64-bit architectures), page replacement algorithms, and virtual-to-physical address translation.
- **System Calls**: Emulates the interface between user programs and the OS kernel, allowing simulated processes to request resources or OS-level operations.
- **Timer & Hardware**: Simulates hardware timer interrupts to enforce time-sharing among processes and basic CPU execution cycles.

---

## How to Build and Run Test Cases

### 1. Build the Simulator
To compile the C source code, use the provided `Makefile`. Open your WSL/Linux terminal and run:
```bash
make clean
make
```
This will compile all `.c` files in `src/` and generate the `os` executable.

### 2. Run a Specific Test Case
To run an individual test case, pass the configuration filename to the `os` executable. 

**General syntax:**
```bash
./os <name_file_test_in_folder_input>
```

**Run individual test cases (copy & paste):**
```bash
./os os_0_mlq_paging
./os os_1_mlq_paging
./os os_1_mlq_paging_small_1K
./os os_1_mlq_paging_small_4K
./os os_1_singleCPU_mlq
./os os_1_singleCPU_mlq_paging
./os os_2_mlq_paging
./os os_2_singleCPU_mlq_paging
./os os_sc
./os os_syscall
./os os_syscall_list
./os sched
./os sched_0
./os sched_1
```

### 3. Run ALL Test Cases

You can run the following bash snippet in your terminal. It automatically iterates through all test configurations in the `input` directory and runs the simulator for each:

```bash
# Loop through all test cases in the input directory
for test_file in input/*; do
    if [ -f "$test_file" ]; then
        # Extract just the filename (e.g., os_1_mlq_paging)
        config_name=$(basename "$test_file")
        
        echo "==========================="
        echo "Testing: $config_name"
        
        # Run the simulator
        ./os "$config_name"
    fi
done
```

---

## Memory Management (MM64) Implementation Details

We have completed the Memory Management module, successfully passing all edge cases and intentional bugs introduced by the assignment specification. Here is a breakdown of the architecture:

### 1. Process Isolation & `krnl` Mapping (`os.c` & `sys_mem.c`)
- **Bug Fixed:** Originally, the loader assigned the global `&os` struct to every process. This caused a critical issue where all processes overwrote each other's memory management (`mm`) pointer.
- **Solution:** We dynamically allocate a private `krnl_t` instance for each process upon loading. Global structures like `ready_queue` and physical memory (`mram`, `mswp`) are passed by reference, while `krnl->mm` is kept strictly isolated for each process.
- **Dynamic PCB Lookup:** When `sys_memmap` is called, it correctly scans the `running_list` queue using the process PID to locate the actual caller PCB instead of using dummy memory.

### 2. Virtual Memory Areas (VMA) (`mm-vm.c` & `libmem.c`)
- **`__alloc` & `sbrk` Update:** Fixed a bug where memory allocation failed to increment the `sbrk` boundary. Now, when a process requests more memory and the `SYSMEM_INC_OP` system call succeeds, the VMA's `sbrk` limit is correctly increased.
- **Overlap Validation:** `validate_overlap_vm_area` accurately ensures that dynamically growing VMAs do not overwrite adjacent memory regions. 

### 3. 5-Level Paging Architecture (`mm64.c`)
- **Hierarchy:** Implemented 64-bit address translation tracking across 5 levels (`PGD -> P4D -> PUD -> PMD -> PT`).
- **PTE Operations (`pte_set_fpn`, `pte_set_swap`, `pte_get_entry`):** 
  - **Intentional Bug Fixed:** The initial code allocated a dummy pointer (`malloc(sizeof(addr_t))`) and wrote the Page Table Entry (PTE) there. The data was immediately lost, resulting in all pages being incorrectly marked as "Not Present" (`pte = 0`).
  - **Solution:** Modified these functions to directly access the physical flat array `caller->krnl->mm->pgd[pgn]`. 

### 4. Page Replacement & FIFO SWAP (`libmem.c`)
- **Page Fault Handling:** If a required page is not in RAM, `pg_getpage` invokes the page replacement algorithm.
- **Victim Selection:** `find_victim_page` employs a Strict FIFO queue to evict the oldest page. We also patched a pointer manipulation bug that broke the FIFO queue when it contained only a single node.
- **PTE Flagging:** When a page is evicted to SWAP, its `PRESENT` bit is explicitly cleared (0) and the `SWAPPED` bit is enabled (1). When brought back to RAM, the flags are properly toggled.

### 5. Smart Configuration Loader (`os.c`)
- **Legacy Compatibility:** Test files like `os_1_singleCPU_mlq` do not provide a memory configuration line (RAM/SWAP sizes), which previously crashed `fscanf`. 
- **Solution:** Implemented a heuristic peek detection. If the file contains a massive number (`> 65536`), the loader interprets it as a memory config line. Otherwise, it safely rewinds and assigns hardcoded legacy memory capacities.

---

## Folder Structure and Descriptions

### `include/`
This folder contains all the C header (`.h`) files that define the data structures, macros, and function prototypes.
- **Hardware & Core**: `cpu.h`, `timer.h`, `mem.h`.
- **Memory Management**: `mm.h`, `mm64.h`, `os-mm.h`, `libmem.h`.
- **Scheduling**: `sched.h`, `queue.h`.
- **System Calls**: `syscall.h`.
- **Utilities**: `common.h`, `os-cfg.h`, `loader.h`, `bitops.h`.

### `src/`
Holds the C source (`.c`) files containing the actual implementation logic.
- **Core OS & CPU**: `os.c`, `cpu.c`, `timer.c`.
- **Memory Management**: `mm.c`, `mm64.c`, `mm-vm.c`, `mm-memphy.c`, `paging.c`, `libmem.c`.
- **Scheduling**: `sched.c`, `queue.c`.
- **System Calls**: `syscall.c`, `sys_mem.c`, `sys_listsyscall.c`.
- **Utilities**: `loader.c`, `libstd.c`.

### `input/`
Contains the configuration files (`os_0_mlq_paging`, etc.) and simulated process programs (inside the `proc/` subdirectory) written in a custom pseudo-instruction format.

### `output/`
Stores the reference execution logs (`.output`) used to verify the correctness of the OS implementation.

---

## Collaboration & Git Workflow

To ensure a smooth collaboration process and avoid merge conflicts, please follow this Git workflow when contributing to the repository:

1. **Keep your local main branch up to date:**
   ```bash
   git checkout main
   git pull origin main
   ```

2. **Create a new branch for your feature or fix:**
   ```bash
   git checkout -b your-feature-branch-name
   ```

3. **Make your changes and commit:**
   ```bash
   git add .
   git commit -m "Description of the changes made"
   ```

4. **Sync with main before pushing:**
   ```bash
   git pull origin main
   ```

5. **Push your branch to the remote repository:**
   ```bash
   git push origin your-feature-branch-name
   ```

6. **Create a Pull Request:**
   Go to the repository platform and open a Pull Request (PR) from your feature branch to the `main` branch. Wait for your collaborators to review and approve the PR before merging.