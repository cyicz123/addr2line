#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include "addr2line.h"

int FuncA();
void FuncB(int argA, char argB);
int FuncC(int argA, float argB, double argC);

void dump()
{
    int size = 0;
    void *buf[256] = {0};
    char **cbuf = NULL;
    char **addresses = NULL;

    size = backtrace(buf, sizeof(buf) / (sizeof(void *)));
    if (!size)
    {
        return;
    }

    addresses = backtrace_symbols(buf, size);
    if (addresses == NULL)
    {
        return;
    }
    char *file_name = (char *)malloc(sizeof(char) * 30);
    get_this_file_name(file_name);

    if (file_bfd_open(file_name, NULL) == error)
    {
        printf("bfd file open failed.");
        free(addresses);
        free(file_name);
        return error;
    }

    unsigned int result = nonsense;
    cbuf = (char **)malloc(size * sizeof(char *));

    for (int i = 0; i < size; i++)
    {
        cbuf[i] = (char *)malloc(sizeof(char[8]));
        sprintf(cbuf[i], "%x", (unsigned int)buf[i]);
        result = translate_address(cbuf[i]);
        printf("%s\n", addresses[i]);
        if (result == error)
        {
            printf("translate address occur error!\n");
        }
        else if (result == no_found)
        {
            printf("Can't find the address.\n");
        }
        else if (result == nonsense)
        {
            printf("The address have found,but everything is null.So it's nonsense.");
        }
        else
        {
            if (result & file_flag)
            {
                printf("file name is %s\n", get_file_name());
            }
            if (result & function_flag)
            {
                printf("function name is %s\n", get_function_name());
            }
            if (result & line_flag)
            {
                printf("line is %u\n", get_line());
            }
            if (result & discriminator_flag)
            {
                printf("discriminator is %u\n", get_discriminator());
            }
            result = ok;
        }
        printf("\n");
    }


    for (int i = 0; i < size; i++)
    {
        free(cbuf[i]);
    }
    free(cbuf);
    free(addresses);
    free(file_name);
    file_bfd_close();
    addresses = NULL;
    return;
}

int main()
{
    FuncA();
    printf("FuncA end.\n");
    return 0;
}

int FuncA()
{
    FuncB(0, 'c');
    printf("FuncB end.\n");
    return 0;
}

void FuncB(int argA, char argB)
{
    FuncC(1, 3.14, 3.14);
    printf("FuncC end.\n");
}

int FuncC(int argA, float argB, double argC)
{
    // backtrace function
    dump();
    return 0;
}