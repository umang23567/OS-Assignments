// Umang
// Vinit

#include "loader.h"
#include <errno.h>

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

// Release memory 
void loader_cleanup() 
{
    if (ehdr) 
    {
        free(ehdr);
    }

    if (phdr) 
    {
        free(phdr);
    }

    if (fd>=0) 
    {
        close(fd);
    }
}

// Load and run the ELF executable file
void load_and_run_elf(char** exe) 
{
    // open the ELF file in ReadOnly Mode
    fd=open(exe[1],O_RDONLY);       
    if (fd<0)                     
    {
        fprintf(stderr,"Failed to open the ELF file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // allocating space for elf header
    ehdr=(Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (!ehdr) 
    {
        fprintf(stderr,"Failed to allocate memory for ELF header: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    read(fd,ehdr,sizeof(Elf32_Ehdr));

    // allocating space for program header
    phdr=(Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
    if (!phdr) 
    {
        fprintf(stderr,"Failed to allocate memory for program headers: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // moving fd to start of program header
    lseek(fd,ehdr->e_phoff,SEEK_SET);   

    // iterate through header table
    for (int i=0;i<ehdr->e_phnum;i++) 
    {
        read(fd,phdr,sizeof(Elf32_Phdr));
        // finding section with type LOAD and entry_point present
        if (phdr->p_type==PT_LOAD && ehdr->e_entry<phdr->p_vaddr+phdr->p_memsz && ehdr->e_entry>=phdr->p_vaddr)
        {
            break;
        }
    }

    // allocate memory for the segment 
    void *segment=mmap(NULL,phdr->p_memsz,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_ANONYMOUS,0,0);
    
    // point to start of that segment
    lseek(fd,phdr->p_offset,SEEK_SET);  
    // read the segment and store in segment memory
    read(fd,segment,phdr->p_memsz);     

    // calclating segment offset 
    void *entry_point=segment+(ehdr->e_entry-phdr->p_vaddr);

    int(*_start)()=(int(*)())entry_point;
    int result=_start();
    printf("User _start return value = %d\n",result);
}

int main(int argc, char** argv) 
{
    if (argc!=2) 
    {
        printf("Usage: %s <ELF Executable>\n",argv[0]);
        exit(1);
    }

    // Carry out necessary checks on the input ELF file
    load_and_run_elf(argv);

    // Cleanup inside the loader
    loader_cleanup();
    return 0;
}