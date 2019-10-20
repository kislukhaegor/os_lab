#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

size_t transliterate(unsigned char* input, size_t input_size, char* output, size_t output_size)
{
    static const char* translit_table[] =
    {
        "yu", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
        "p", "ya", "r", "s", "t", "u", "zh", "v", "'", "y", "z", "sh", "e", "shch", "ch", "\"",
        "YU", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
        "P", "YA", "R", "S", "T", "U", "ZH", "V", "'", "Y", "Z", "SH", "E", "SHCH", "CH", "\"",

    };

    if (input == NULL || output == NULL)
    {
        return 0;
    }

    if (output_size == 0)
    {
        return 0;
    }
    size_t input_i = 0;
    size_t output_i = 0;
    output[output_i] = '\0';
    for (; input_i < input_size && output_i < output_size; ++input_i)
    {
        unsigned codepoint_off = input[input_i] - 192;
        if (codepoint_off > 63)
        {
            output[output_i++] = input[input_i];
            continue;
        }

        size_t codepoint_size = strlen(translit_table[codepoint_off]);

        if (output_i + codepoint_size >= output_size + 1)
        {
            break;
        }
        for (unsigned i = 0; i < codepoint_size; ++i)
        {
            output[output_i + i] = translit_table[codepoint_off][i];
        }
        output_i += codepoint_size;
    }
    output[output_i++] ='\0';
    return input_i;
}

int main(int argc, char **argv)
{
    static unsigned char input_buf[BUFSIZ];
    static unsigned char output_buf[BUFSIZ];

    size_t read_lines = 0; // количество прочитанных байт из stdin
    while ((read_lines = read(STDIN_FILENO, input_buf, BUFSIZ)) > 0)
    {
        size_t processed = 0;
        do {
            processed += transliterate(input_buf, read_lines, output_buf, BUFSIZ);
            if (processed > 0)
            {
                write(STDOUT_FILENO, output_buf, strlen(output_buf));
            }
        }
        while (processed < read_lines);
    }
    return 0;
}
