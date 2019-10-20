#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


typedef int (*FCMP)(const void*, const void*);

static char* list[512];

int compare(const void* s1, const void* s2) {
    return strcmp(*((const char**) s1), *((const char**) s2));
}

int diag(char *mes) {
    write(1, mes, strlen(mes));
    write(1, ": ", 2);
    write(1, sys_errlist[errno], strlen(sys_errlist[errno]));
    write(1, "\n", 1);
    exit(errno);
}

int scanfolder() {
    DIR* fdir;
    struct dirent* folder;
    struct stat sbuf[1];
    int count = 0;

    if ((fdir = opendir(".")) == NULL) {
        return (errno);
    }

    char* start = sbrk(0);
    while ((folder = readdir(fdir)) != NULL) {
        // Вытаскиваем инфу об элементе папки
        stat(folder->d_name, sbuf);

        int len = strlen(folder->d_name);
        // Выделяем память под название папк

        if (!((sbuf->st_mode) & S_IXUSR) || ((sbuf->st_mode) & S_IFDIR))
        {
            continue;
        }
        char* s = sbrk(len + 2);
        memcpy(s, folder->d_name, len);
        memcpy(s + len, "\n", 1);
        list[count] = s;
        count++;
    }
    closedir(fdir);
    qsort(list, count, sizeof(char*), (FCMP)compare);

    for (int i = 0; i < count; i++) {
        int len = strlen(list[i]);
        write(1, list[i], len);
    }

    brk(start);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return (diag("Specify directory"));
    }

    if (chdir(argv[1]) < 0) {
        return (diag(argv[1]));
    }
    
    if (scanfolder() > 0) {
        return (diag(argv[1]));
    }
    return (0);
}
