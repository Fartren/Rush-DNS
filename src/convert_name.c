// cc -D TEST src/convert_name.c -o convert_name.exe && ./convert_name.exe
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char *convert_name(char *name)
{
    size_t to_analyze = strlen(name);
    char *ret =
        malloc(to_analyze + 2); // last 0 and first digit for first component
    char *current = ret;
    while (to_analyze > 0)
    {
        char *dot = memchr(name, '.', to_analyze);
        if (dot == NULL)
        {
            current[0] = (char)to_analyze;
            ++current;
            strncpy(current, name, to_analyze);
            current[to_analyze] = 0;
            break;
        }
        size_t comp_size = dot - name;
        current[0] = (char)comp_size;
        ++current;
        strncpy(current, name, comp_size);
        current += comp_size;
        name = dot + 1;
        to_analyze -= comp_size + 1;
    }
    return ret;
}

#ifdef TEST
#    include <stdio.h>

int main(void)
{
#    define EXPECT(name, expected)                                             \
        do                                                                     \
        {                                                                      \
            if (strncmp(convert_name(name), expected,                          \
                        sizeof(expected) / sizeof(expected))                   \
                == 0)                                                          \
            {                                                                  \
                printf("test passed for: %s\n", name);                         \
            }                                                                  \
            else                                                               \
            {                                                                  \
                printf("test failed for: %s\n", name);                         \
            }                                                                  \
        } while (0)
    EXPECT("test.com", "\x04test\x03com\x0");
    EXPECT("test.fr", "\x04test\x02fr\x0");
    EXPECT("www.google.com", "\x03www\x05google\x03com\x0");
    EXPECT("example.com", "\x7example\x3com\x0");
}
#endif
