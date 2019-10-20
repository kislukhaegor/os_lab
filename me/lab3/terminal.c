#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>

#define MAX_NUMBER 1000
#define MAX_NUMBER_LEN 4

#define PRINT_STATIC_STRING(str)\
    write(1, str, sizeof(str))

#define PRINT_STRING(str)\
    write(1, str, strlen(str))

typedef struct expression {
    unsigned left_op;
    unsigned right_op;
} expression_t;

int reverse_string(char* str, size_t str_len) {
    if (str == NULL) {
        return -1;
    }

    for (size_t i = 0; i < str_len / 2; ++i) {
        char tmp = str[i];
        str[i] = str[str_len - i - 1];
        str[str_len - i - 1] = tmp;
    }
    return 0;
}

int num_to_string(long num, char* buf) {
    if (buf == NULL) {
        return -1;
    }
    if (num == 0) {
        buf[0] = '0';
        return 0;
    }
    int i = 0;
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    reverse_string(buf, i);
    buf[i] = '\0';
    return 0;
}

int textmode(int mode) {
    static struct termios con[2];
    tcgetattr(0, &con[0]);
    if(mode > 0) {
        return(tcsetattr(0, TCSAFLUSH, &con[1]));
    }
    tcgetattr(0, &con[1]);
    con[0].c_lflag &= ~(ICANON | ECHO | ISIG);
    con[0].c_iflag &= ~(ISTRIP | IXOFF | IXANY | IXON);
    con[0].c_oflag |= CS8;
    con[0].c_cc[VMIN] = 2;
    con[0].c_cc[VTIME] = 1;
    return tcsetattr(0, TCSAFLUSH, &con[0]);
}

int getch() {
    unsigned char c[2] = {};
    
    static int len = 0;
    if ((len = read(0, c, 2)) < 2) {
        return c[0];
    }
    if(c[0] == 27) {
        c[0] = 0;
    }

    ungetc(c[1], stdin);
    return c[0];
}

int rand_expr(expression_t* expr) {
    if (expr == NULL) {
        return -1;
    }
    srand(getpid());
    expr->left_op = rand() % MAX_NUMBER;
    expr->right_op = rand() % MAX_NUMBER;
    return 0;
}

void print_diag(time_t seconds, int err_count) {
    static char buf[128];
    PRINT_STATIC_STRING("\nSpent time: ");
    num_to_string(seconds, buf);
    PRINT_STRING(buf);
    if (seconds == 1) {
        PRINT_STATIC_STRING(" second\n");
    } else {
        PRINT_STATIC_STRING(" seconds\n");
    }
    PRINT_STATIC_STRING("Number of mistakes: ");
    num_to_string(err_count, buf);
    PRINT_STRING(buf);
    PRINT_STATIC_STRING("\n");
}

int main(int argc, char* argv[]) {
    static char buf[MAX_NUMBER_LEN + 1];

    expression_t expr;
    rand_expr(&expr);

    num_to_string(expr.left_op, buf);
    PRINT_STRING(buf);
    PRINT_STATIC_STRING(" + ");
    num_to_string(expr.right_op, buf);
    PRINT_STRING(buf);
    PRINT_STATIC_STRING("\n");
    textmode(0);

    num_to_string(expr.left_op + expr.right_op, buf);
    int len = strlen(buf);

    time_t start = time(NULL);
    int err = 0;
    int i = 0;
    while(i < len) {
        unsigned char c = 0;
        switch(c = getch()) {
        case 0:
            c = '\007';
            switch(getch()) {
            case 67:
                c = buf[i];
                break;
            case 68:
                if(i == 0) {
                    break;
                }
                i--;
                write(1, "\b", 1);
                continue;
            default:
                break;
            }
            break;
        case 27:
            i = len;
            c = '\007';
            break;
        default:
            if(c != buf[i]) {
                c = '\007';
            }
            break;
        }
        (c == '\007') ? err++ : i++;
        write(1, &c, 1);
    }
    print_diag(time(NULL) - start, err);

    textmode(1);
    return 0;
}
