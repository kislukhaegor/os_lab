#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

static unsigned char buf[BUFSIZ];

void recode(size_t buf_size)
{
    unsigned char c = 0;

    static char tab[32] = {
        // ю,  а, б,  ц, д, е,  ф, г,  х, и, й,  к,  л,  м,  н,  о
           30, 0, 1, 22, 4, 5, 20, 3, 21, 8, 9, 10, 11, 12, 13, 14,
        // п,   я,  р,  с,  т,  у, ж, в,  ь,  ы, з,  ш,  э,  щ,  ч,  ъ
           15, 31, 16, 17, 18, 19, 6, 2, 28, 27, 7, 24, 29, 25, 23, 26
    };

    for (int i = 0; i < buf_size; i++) {
        c = buf[i];
        if (c < 192) {  // ASCII
            continue;
        }
        if (c < 224) {  // Lower KOI 192-223
            buf[i] = tab[c - 192] + 224;
        } else {        // Upper KOI 224-255
            buf[i] = tab[c - 224] + 192;
        }
    }
}

void display_usage(const char* program_name)
{
    static const char usage_info_part1[] = "Usage: ";
    static const char usage_info_part2[] =
    " source_file target_file [-f]\n"\
    " -f    force rewrite target file\n";
    write(STDERR_FILENO, usage_info_part1, sizeof(usage_info_part1));
    write(STDERR_FILENO, program_name, strlen(program_name));
    write(STDERR_FILENO, usage_info_part2, sizeof(usage_info_part2));

}

int diag(const char* name, const char* mes)
{
    int len = 0;    // Общая длина диагностической строки вывода
    strcpy(buf, name);
    strncat(buf, ": ", 2);
    strcat(buf, mes);
    len = strlen(buf);
    buf[len] = '\n';
    write(STDERR_FILENO, buf, len + 1);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        display_usage(argv[0]);
        return 127;
    }

    int force_write = 0;
    if (argc > 3)
    {
        if (strcmp(argv[3], "-f") == 0)
        {
            force_write = 1;
        } else
        {
            display_usage(argv[0]);
            return 127;
        }
    }

    int fd_source = open(argv[1], O_RDONLY);
    if (fd_source < 0)
    {
        diag(argv[1], sys_errlist[errno]);
        return errno;
    }

    int fd_target = 0;
    if (access(argv[2], F_OK) == 0)
    {
        if (force_write)
        {
            fd_target = open(argv[2], O_WRONLY);
            if (fd_target < -1)
            {
                diag(argv[2], sys_errlist[errno]);
                close(fd_source);
                return errno;
            }
        }
        else
        {
            diag(argv[2], "file already exists. Use -f option to rewrite");
            close(fd_target);
            close(fd_source);
            return errno;
        }
    } else
    {
        fd_target = creat(argv[2], 0644);
        if (fd_target < 0)
        {
            diag(argv[2], sys_errlist[errno]);
            close(fd_source);
            return errno;
        }
    }

    size_t read_lines = 0;
    while ((read_lines = read(fd_source, buf, BUFSIZ)) > 0)
    {
        recode(read_lines);
        write(fd_target, buf, read_lines);
    }

    close(fd_target);
    close(fd_source);
    return 0;
}
