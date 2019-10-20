/* The dice */

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <time.h>

void clrscr() {
    printf("\033[H");
    printf("\033[2J");
}

static int done[2] = {};

typedef void (*sighandler)(int);
void brake(int);
jmp_buf pos;
static char digit[3];
static char* who[] = { "Com1", "Com2"};

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s timeout1 timeout2\n", argv[0]);
        return 0;
    }

    int timeouts[2] = {};
    for (int i = 0; i < 2; ++i) {
        if((timeouts[i] = atoi(argv[i + 1])) < 1) {
            timeouts[i] = 1;
        }
    }

    int values[2] = {getpid() % 6, time(NULL) % 6};
    int j = 0;
    signal(SIGALRM, (sighandler) brake);
    int max_timeout = 0;
    if (timeouts[1] > timeouts[0]) {
        max_timeout = 1;
    }

    alarm(timeouts[max_timeout]);
    j = setjmp(pos);
    if (j == 1) {
        int timeout_diff = timeouts[max_timeout] - timeouts[(max_timeout + 1) % 2];
        if (timeout_diff == 0) {
            j = 2;
        } else {
            alarm(timeout_diff);
        }
    }
    while(j < 2) {
        clrscr();
        for (int i = 0; i < 2; ++i) {
            if (!done[i]) {
                values[i] = (values[i] + 1) % 6;
            }
            printf("%s: %d\n", who[i], values[i]);
        }
        usleep(50000);
    } /* while-j */
    signal(SIGINT, SIG_DFL);
    if (values[0] > values[1]) {
        printf("Com1 is winner\n");
    } else if (values[1] > values[0]) {
        printf("Com2 is winner\n");
    } else {
        printf("Draw\n");
    }
    exit(0);
}

void brake(int c) {
    static int j = 0;
    done[j] = 1;
    longjmp(pos, ++j);
    return;
}
