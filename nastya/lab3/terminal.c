#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>

#define MAX_NUMBER 1000

#define PRINT_STATIC_STRING(str)\
    write(1, str, sizeof(str))

#define PRINT_STRING(str)\
    write(1, str, strlen(str))

typedef struct expression {
    int left_op;
    int right_op;
} expression_t;

int reverse_string(char* str, size_t str_len) { // разворот строки
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

int num_to_string(long num, char* buf) { // приводим число к строке
    if (buf == NULL) {
        return -1;
    }
    if (num == 0) {
        buf[0] = '0';
        return 0;
    }
    int i = 0;
    if (num < 0)
    {
        buf[i] = '-';
        buf += 1;
        num *= -1;
    }
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    buf[i] = '\0';
    reverse_string(buf, i);
    return 0;
}

int textmode(int mode) { //выставляет режим терминала
    static struct termios con[2];
    tcgetattr(0, &con[1]);
    if(mode > 0) {
        return(tcsetattr(0, TCSAFLUSH, &con[1]));
    }
    tcgetattr(0, &con[0]);
    con[0].c_lflag &= ~(ICANON | ECHO | ISIG);
    con[0].c_iflag &= ~(ISTRIP | IXOFF | IXANY | IXON);
    con[0].c_oflag |= CS8;
    con[0].c_cc[VMIN] = 2;
    con[0].c_cc[VTIME] = 1;
    return tcsetattr(0, TCSAFLUSH, &con[0]);
}

int getch() {
    unsigned char c[1] = {};
    
    static int len = 0;
    read(0, c, 1);
    return c[0];
}

int rand_expr(expression_t* expr) {
    if (expr == NULL) {
        return -1;
    }
    srand(getpid()); // генерирует случайную последовательность
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

int parse_num()
{
    int sign = 0;
    int num = 0;
    char c = 0;
    while (read(0, &c, 1) > 0)
    {
        if (c >= '0' && c <= '9')
        {
            num = num * 10 + c - '0';
        }
        else if (c == '-' && !sign)
        {
            sign = 1;
        }
        else
        {
            break;
        }
    }
    num = sign == 0 ? num : num * -1;
    return num;
}

int main() {
    static char buf[BUFSIZ];
    PRINT_STATIC_STRING("Input first operand: ");
    int left_op = parse_num();
    PRINT_STATIC_STRING("\n");
    expression_t expr;
    expr.left_op = left_op;
    rand_expr(&expr);

    num_to_string(expr.left_op, buf);
    PRINT_STRING(buf);
    PRINT_STATIC_STRING(" - ");
    num_to_string(expr.right_op, buf);
    PRINT_STRING(buf);
    PRINT_STATIC_STRING("\n");
    textmode(0);
    num_to_string(expr.left_op - expr.right_op, buf);
    int len = strlen(buf);

    time_t start = time(NULL);
    int err = 0;
    int i = 0;
    while(i < len) {
        unsigned char c = 0;
        switch(c = getch()) {
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
