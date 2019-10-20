#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


int is_same_files(int fd1, int fd2)
{
    struct stat stat1, stat2;
    if (fstat(fd1, &stat1) < 0)
    {
        return -1;
    }
    if (fstat(fd2, &stat2) < 0)
    {
        return -1;
    }
    return (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino);
}

int diag(char *mes) {
    write(1, mes, strlen(mes));
    if (errno)
    {
        write(1, ": ", 2);
        write(1, sys_errlist[errno], strlen(sys_errlist[errno]));
    }
    write(1, "\n", 1);
    exit(errno);
}


int copy_file(const char* source, const char* dest)
{
    static char buf[BUFSIZ];
    if (source == NULL || dest == NULL)
    {
        return -1;
    }

    int fd_src = open(source, O_RDONLY);
    if (fd_src < 0)
    {
        return -1;
    }

    int fd_dest = open(dest, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd_dest < 0)
    {
        close(fd_src);
        return -1;
    }

    size_t readed = 0;
    while ((readed = read(fd_src, buf, BUFSIZ)) > 0)
    {
        if (write(fd_dest, buf, readed) < 0)
        {
            return errno;
        }

    }
    close(fd_src);
    close(fd_dest);
    return 0;
}

int copy_folder(const char* dest_folder) {
    static char buf[1024];
    size_t dest_len = strlen(dest_folder);

    DIR* cur_dir = NULL;
    if ((cur_dir = opendir(".")) == NULL) {
        return (errno);
    }

    if (mkdir(dest_folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
    {
        if (errno != EEXIST)
        {
            printf("%d %d\n", EEXIST, errno);
            return errno;
        }
        else
        {
            int fd = open(dest_folder, O_DIRECTORY);
            if (fd ==-1)
            {
                return errno;
            }
            switch (is_same_files(dirfd(cur_dir), fd))
            {
                case -1:
                    return errno;
                case 1:
                    return 0;
                default:
                    break;
            }
        }
    }

    struct dirent* file;
    struct stat sbuf[1];
    while ((file = readdir(cur_dir)) != NULL) {
        // Вытаскиваем инфу об элементе папки
        stat(file->d_name, sbuf);
        if (S_ISREG(sbuf->st_mode))
        {
            strcpy(buf, dest_folder);
            strcpy(buf + dest_len, "/");
            strcpy(buf + dest_len + 1, file->d_name);
            if (copy_file(file->d_name, buf) != 0)
            {
                if (errno != EEXIST)
                {
                    closedir(cur_dir);
                    return errno;
                }
            }
        }
    }
    closedir(cur_dir);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return (diag("Specify directory"));
    }
    
    if (copy_folder(argv[1]) != 0) {
        return (diag(argv[1]));
    }
    return (0);
}
