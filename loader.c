// Assignment-4
// Simple Smart Loader 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <elf.h>
#include <signal.h>
#include <errno.h>

Elf32_Ehdr *ehdr;              // pointer to ELF header
Elf32_Phdr *phdr;              // pointer to program headers
int fd;                        // file descriptor for the ELF file

size_t default_page_size = 4096;        // page size for mmap
int page_fault_count = 0;               // counter for page faults
int page_allocation_count = 0;          // counter for page allocations
size_t internal_fragmentation = 0;      // total size of internal fragmentation


// release memory 
void loader_cleanup() 
{
    if (ehdr) free(ehdr); 
    if (phdr) free(phdr);
    if (fd>=0) close(fd); 
}


/*
page fault handler-
sig: signal type
info: structure containing information about the signal 
context: context of the process at the time of the signal 
*/ 
void handle_page_fault(int sig, siginfo_t *info, void *context)
{
    void *fault_addr = info->si_addr;                   // address at which fault occurs is extracted
    printf("Page fault at address: %p\n", fault_addr);

    // check if the fault address falls within a valid segment
    for (int i = 0; i < ehdr->e_phnum; i++)                                 // to itreate through program header
    {
        if (phdr[i].p_type == PT_LOAD &&                                    // PT_Load type segment
            fault_addr >= (void*)phdr[i].p_vaddr &&                         // within p_vaddr
            fault_addr < (void*)(phdr[i].p_vaddr + phdr[i].p_memsz))        // and within p_memsz
        {
            // calculate the page-aligned address
            uintptr_t aligned_addr = (uintptr_t)fault_addr & ~(default_page_size - 1);      // align to nerest page boundary addresss

            // check if the page has already been allocated
            if ((uintptr_t)aligned_addr >= (uintptr_t)phdr[i].p_vaddr + phdr[i].p_memsz) 
            {
                fprintf(stderr, "Accessing memory beyond segment size\n");
                exit(EXIT_FAILURE);
            }

            // map a single page
            void *page = mmap((void*)aligned_addr, default_page_size,                                // map single page size
            PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
            if (page == MAP_FAILED)                                                                  // mmap error
            {
                perror("mmap failed");
                exit(EXIT_FAILURE);
            }

            page_allocation_count++;                                                            // page successfully allocated

            // load the corresponding page from the ELF file
            size_t offset = aligned_addr - phdr[i].p_vaddr + phdr[i].p_offset;
            lseek(fd, offset, SEEK_SET);                                                // move fd to offset
            read(fd, page, default_page_size);                                       // read for only page size

            // Calculate the required number of pages for this segment
            size_t segment_size = phdr[i].p_memsz;
            size_t pages_needed=(size_t)((double)(segment_size+default_page_size-1)/default_page_size);
            size_t allocated_size = pages_needed * default_page_size;
          
            // Calculate fragmentation for this segment only if it's the first page fault in the segment
            if (page_allocation_count % pages_needed == 0) 
            {
              size_t fragmentation = allocated_size - segment_size;
              internal_fragmentation += fragmentation;
            }


            page_fault_count++;                                                        //  page fault resolved

            printf("Allocated page for fault address 0x%p at memory location 0x%p\n", fault_addr, page);
            printf("Page start: 0x%zx, Page offset: 0x%zx\n", aligned_addr, offset);
           printf("Segment size: %zu, Size allocated: %zu\n", phdr[i].p_memsz, default_page_size);

            return;  // return to resume program execution
        }
    }

    // if the address doesn't match any segment, then terminate 
    fprintf(stderr, "Segmentation fault at invalid address: %p\n", fault_addr);
    exit(EXIT_FAILURE);
}

// load and run the ELF executable file
void load_and_run_elf(char** exe) 
{
    // open the ELF file in ReadOnly Mode
    fd = open(exe[1], O_RDONLY);       
    if (fd < 0)                     
    {
        fprintf(stderr, "Failed to open the ELF file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // allocate space for ELF header 
    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (!ehdr) 
    {
        fprintf(stderr, "Failed to allocate memory for ELF header: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // read the ELF header
    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) == -1)
    {
        fprintf(stderr, "Error reading ELF header: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // allocate space for program headers
    phdr = (Elf32_Phdr *)malloc(ehdr->e_phnum * sizeof(Elf32_Phdr));
    if (!phdr) 
    {
        fprintf(stderr, "Failed to allocate memory for program headers: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // move to the start of the program header table
    if (lseek(fd, ehdr->e_phoff, SEEK_SET) == -1) 
    {
        fprintf(stderr, "Error seeking to program header table: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // read program header table
    // size_t bytes_read = read(fd, phdr, ehdr->e_phnum * sizeof(Elf32_Phdr));
    if (read(fd, phdr, ehdr->e_phnum * sizeof(Elf32_Phdr)) == -1)
    {
        fprintf(stderr, "Error reading program header table: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // set up signal handler for segmentation faults
    struct sigaction sa;                                    // declaring variable "sa" for signal handling (segmentation fault)
    sa.sa_flags = SA_SIGINFO;                               // to receive more detailed info (i.e. address where fault occurred)
    sa.sa_sigaction = handle_page_fault;                    // calling our customer signal handler (for page fault)
    sigemptyset(&sa.sa_mask);                               // to not block any other signal while our handler is running
    sigaction(SIGSEGV, &sa, NULL);                          // call sigaction to install the custom handler

    // attempt to start execution at the entry point, expecting page faults
    int (*_start)() = (int(*)())(ehdr->e_entry);
    int result = _start();

    // Output the result of execution
    printf("User _start return value = %d\n", result);
    printf("Total number of page faults: %d\n", page_fault_count);
    printf("Total number of page allocations: %d\n", page_allocation_count);
    printf("Total internal fragmentation: %zu byte\n", internal_fragmentation );    // Output in byte
}

int main(int argc, char** argv) 
{
    if (argc != 2) 
    {
        printf("Usage: %s <ELF Executable>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Run executable
    load_and_run_elf(argv);  

    // Clean up 
    loader_cleanup();        

    return 0;
}
