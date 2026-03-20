#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <stdlib.h>

int dummy_main(int argc, char** argv);

int main(int argc, char ** argv){
    int ret= dummy_main(argc, argv);
    return ret;
}

#define main dummy_main