#include <unistd.h>
#include <stdio.h>
#include <sys/timeb.h>

/* clear standart text mode window */

void clrscr() {
    static unsigned char esc[11];

    /* obtain set cursor to left-top window corner */

    esc[0] = 27;
    esc[1] = '[';
    esc[2] = 'H';
    write(1, esc, 3);

    /* obtain clear escape sequence */

    esc[2] = '2'; esc[3] = 'J';
    write(1,esc,4);
}

/* position cursor in standart text window */

void gotoxy(int tx, int ty, char c) {
    unsigned char esc[16];
    static unsigned char ystr[3]; /* vertical cursor location */
    static unsigned char xstr[3]; /* horizontal cursor location */
    int i;                        /* ESC-sequence current index */
    int j;                        /* cursor location current index */

    /* convert cursor location to text format */

    if ((tx > 99) || (ty > 99))
      tx = ty = 99;
    if ((tx < 1) || (ty < 1))
      tx = ty = 1;
    xstr[0] = xstr[1] = xstr[2] = '\0';
    ystr[0] = ystr[1] = ystr[2] = '\0';

    sprintf((char *)xstr,"%d",tx);
    sprintf((char *)ystr,"%d",ty);

    /* obtain goto escape sequence */

    esc[0] = 27;
    esc[1] = '[';
    i = 2; j = 0;
    while (ystr[j]) {
        esc[i++] = ystr[j++];
    }
    j = 0;
    esc[i++] = ';';
    while (xstr[j]) {
        esc[i++] = xstr[j++];
    }
    esc[i++]='H';
    esc[i++] = c;
    write(1, esc, i);
}

int main(int argc, char* argv[]) {
    int y = 1;
    int status;
    int i;
    int j = 0;
    int PROCNUM = atoi(argv[1]);
    int* pid = sbrk((PROCNUM + 1) * sizeof(int));
    char* lead = sbrk((PROCNUM + 1) * sizeof(char));
    int dist = atoi(argv[2]);
    int p;
    char bell = '\007';
    struct timeb tp[1];
    int jump;

    clrscr();
    while (j < PROCNUM) {
        if ((pid[j] = fork()) == 0) {
            usleep((PROCNUM - j));
            while (y < dist) {
                gotoxy(j + 1, y, 'A' + j);
                if (y > 1) {
                    gotoxy(j + 1, y - 1, ' ');
                }
                ftime(tp);
                if ((tp[0].millitm % (j + 'A')) != j) {
                    continue;
                }
                y++;
                for(i=0; i<1000000; i++);
            }
            exit('A' + j);
        }
        j++;
    }

    j = 0;
    while ((p = wait(&status)) != (-1)) {
        for(i = 0; i < PROCNUM; i++) {
            if (pid[i] == p) {
                lead[j++] = (char) ((status >> 8) & '\377');
            }
        }
        write(1, bell, 1);
    }
    lead[j] = '\n';
    sleep(1);
    gotoxy(1, dist + 3, '\n');
    write(1, lead, PROCNUM + 1);
    exit(0);
}
