# addr2line
一个提取自linux addr2line命令的c语言库。能够将可执行文件16进制字符串型地址解析出对应文件名、函数名和源码行号。

## 一、例程
一个在屏幕输出调用dump()函数的调用栈，以及对应的文件名、函数名和源码行号的程序。
``` c
void dump()
{
    int size = 0;
    void *buf[256] = {0};
    char **cbuf = NULL; //存储16进制地址字符串
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
    char *file_name = (char *)malloc(sizeof(char) * 30); //确保当执行文件名过长时，不会溢出。
    get_this_file_name(file_name);//获得当前进程名称

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
        sprintf(cbuf[i], "%x", (unsigned int)buf[i]); //将地址转化为十六进制字符串类型
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
    file_bfd_close();//记得关闭
    addresses = NULL;
    return;
}

```

## 二、enum变量
``` c
const enum add2line_status{ok,error};

const enum translate_address_result{no_found=0x10,nonsense=0x100,file_flag=0x1000,function_flag=0x10000,line_flag=0x100000,discriminator_flag=0x1000000};

```

## 三、可调用的函数
### 1. file_bfd_open
``` c
int file_bfd_open(const char *file_name, const char *section_name) 
```
**功能**

* [x] 必需

初始化bfd

**传入参数**
| 参数名    | 类型       | 备注                     |
| ------------ | ------------ | -------------------------- |
| file_name    | const char * | 需要解析的可执行文件名 |
| section_name | const char * | 需要解析的段名，一般为NULL |

**传出参数**

``` c
int \\ok或者error
```

**例程**
``` c
if(file_bfd_open("bt",NULL)==error)
{
    printf("bfd file open failed.");
    return error;
}
```


### 2. file_bfd_close
``` c
void file_bfd_close() 
```
**功能**

* [x] 必需

用了`file_bfd_open`成功后，则必须使用`file_bfd_close`

### 3. get_this_file_name
``` c
int get_this_file_name(char* file_name);
```
**功能**

* [ ] 可选

获取当前进程的名称

**传入参数**
| 参数名    | 类型       | 备注                     |
| ------------ | ------------ | -------------------------- |
| file_name    | char * | 需要获取当前进程的名称 |

**传出参数**

``` c
int \\file_name的长度
```

**例程**
``` c
char *file_name = (char *)malloc(sizeof(char) * 30);
get_this_file_name(file_name);

...

free(file_name)//记得free
```


### 4. translate_address
``` c
unsigned int translate_address(const char* addr);
```
**功能**

* [x] 必需

将传入的地址，解析出需要的一些信息。

**传入参数**
| 参数名    | 类型       | 备注                     |
| ------------ | ------------ | -------------------------- |
| addr    | const char * | 需要解析的地址 |

**传出参数**

``` c
unsigned int \\解析结果用7个标志位来表现各种结果
```

**例程**
``` c
unsigned int result=translate_address("0400837");
  
if(result==error)
{
    printf("error!\n");
}
else if(result==no_found)
{
    printf("Can't find.\n");    
}
else if(result==nonsense)
{
    printf("Have found,but everything is null.So it's nonsense.");
}
else
{
    if(result&file_flag)
    {
        printf("file name is %s\n",get_file_name());
    }
    if(result&function_flag)
    {
        printf("function name is %s\n",get_function_name());
    }
    if(result&line_flag)
    {
        printf("line is %u\n",get_line());
    }
    if(result&discriminator_flag)
    {
        printf("discriminator is %u\n",get_discriminator());
    }
    result=ok;
}
```