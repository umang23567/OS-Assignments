
## Overview:

The ELF loader reads the target ELF file, allocates pages for executable segments as needed, and transfers control to the entry point of the executable. This process involves:

 1. Intercepting page faults with a custom handler
 2. Allocating and mapping pages as the executable needs them
 3. Recording and reporting the number of page faults, allocations, and internal fragmentation after the program execution.

## Key Concepts:

   1.ELF Header (ELF Executable Format): ELF (Executable and Linkable Format) is a common standard for executable files in Unix-like   systems. The ELF header provides metadata about the executable.

   2.Program Headers: These headers in the ELF specify the program's memory layout, the locations to be loaded, and access permissions.

   3.Page Fault Handling: Page faults are triggered when memory accesses are made to unallocated pages. The custom handler intercepts these faults, allocates pages, and maps them into memory.

### Execution Flow:

 The program begins by loading the ELF file into memory, reading the ELF and program headers to locate executable segments and their memory requirements. It then sets up a signal handler for segmentation faults (SIGSEGV) to handle page faults dynamically. During execution, when a page fault is detected, the handler allocates memory pages as needed, loads the corresponding ELF segment data, and resumes execution. Upon completion, it reports statistics on page usage, internal fragmentation, and page faults.

## Detailed Code Walkthrough

### Key Components

 1. **Variables**:
   - `ehdr` and `phdr` store pointers to the ELF header and program headers, respectively, allowing easy access to the executable’s layout.
   - `default_page_size` defines the size of each memory page (4 KB).
   - `page_fault_count` and `page_allocation_count` keep track of page faults and allocations.
   - `internal_fragmentation` tracks unused memory within the allocated pages.

 2. **Functions**:
   - `loader_cleanup()` releases allocated memory and closes the ELF file descriptor.
   - `handle_page_fault()` acts as a custom signal handler that detects page faults, checks if the fault address belongs to a valid segment, and allocates a new page if necessary. It calculates the page-aligned address, maps a new page, reads the appropriate segment data into memory, and updates the page fault counter and internal fragmentation.

 3. **Execution**:
   - The `load_and_run_elf()` function opens the ELF file, reads the ELF and program headers, and sets up the `SIGSEGV` handler for custom page fault handling. It then initiates the executable by calling its entry point and outputs statistics after completion.

 4. **Custom Page Fault Handling**:
   - In `handle_page_fault()`, when a page fault occurs, the function calculates the page-aligned address and allocates a page if the address is within a valid memory segment (`PT_LOAD` segment). It maps one page at a time, reads data from the ELF file for that segment, and calculates any unused space, updating `internal_fragmentation` to track this wasted space. If the fault address lies outside of any segment, it triggers a segmentation fault.

### Main Function and Execution

The main function checks for a command-line argument specifying the ELF file, then calls `load_and_run_elf()` to load and run the file. Finally, it calls `loader_cleanup()` to release resources. The program exits if any errors occur, such as failure to allocate memory, read the ELF file, or map pages. If execution succeeds, it outputs the result of the entry point execution and statistics for page faults, page allocations, and internal fragmentation.



### Output

 Output for ./loader fib
  Page fault at address: 0x8049049
  Allocated page for fault address 0x0x8049049 at memory location 0x0x8049000
  Page start: 0x8049000, Page offset: 0x1000
  Segment size: 114, Size allocated: 4096
  User _start return value = 102334155
  Total number of page faults: 1
  Total number of page allocations: 1
  Total internal fragmentation: 3982 byte

 Output for ./loader sum
  Allocated page for fault address 0x0x8049000 at memory location 0x0x8049000
  Page start: 0x8049000, Page offset: 0x1000
  Segment size: 110, Size allocated: 4096
  Page fault at address: 0x804c000
  Allocated page for fault address 0x0x804c000 at memory location 0x0x804c000
  Page start: 0x804c000, Page offset: 0x3000
  Segment size: 4112, Size allocated: 4096
  Page fault at address: 0x804d000
  Allocated page for fault address 0x0x804d000 at memory location 0x0x804d000
  Page start: 0x804d000, Page offset: 0x4000
  Segment size: 4112, Size allocated: 4096
  User _start return value = 2048
  Total number of page faults: 3
  Total number of page allocations: 3
  Total internal fragmentation: 8066 byte


### Contributions
 Umang Aggarwal- implementation
 Vinit Kumar- testing and readme

##GitHub link: https://github.com/vinit23599/OS_Project_4