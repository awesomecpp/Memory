#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void cat(int fd, int size) {
    char* buf;
    buf = (char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    write(STDOUT_FILENO, buf, size);
}

int main(int argc, char** argv) {
    int fd;
    struct stat st;

    if(argc != 2) {
       printf("cat <filename>\n"); 
       _exit(0);
    }

    if((fd = open(argv[1], O_RDONLY, 0)) == 0) {
        printf("can't open file.");
        _exit(0);
    }

    fstat(fd, &st); 

    cat(fd, st.st_size);

    _exit(0);
}
