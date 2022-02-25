#include <stdio.h>
#include <stdint.h>
// #include "config.h"
#include <bfd.h>
#include <strings.h>
#include <linux/elf.h>
 
/* 
   这里定义3个static变量，并把他们放到一个单独的section中。
   后面，我们通过bfd找出这个section，并得到这3个变量的内容。
   同时，我们还通过符号查找操作，找到a_haha这个static变量的信息。
*/
static uint64_t  a_haha   __attribute__((section ("my_test_sec"))) =3;
static uint64_t  b        __attribute__((section ("my_test_sec"))) =7;
static uint64_t  c        __attribute__((section ("my_test_sec"))) =8;
 
/* 获取当前进程自己的elf文件路径 */
int get_self_path(char *buf, int buf_len)
{
    int ret = readlink("/proc/self/exe", buf, buf_len);
    buf[ret]='\0';
    return ret; 
}
 
void section_proc(bfd *abfd,  asection *sect, PTR obj)
{
    if (strcmp(sect->name, "my_test_sec")==0)
        printf("section %s exists\n", sect->name);
}
 
 
 
void search_a_given_symbol(bfd *ibfd, const char *name)
{
         long storage_needed;
         asymbol **symbol_table;
         long number_of_symbols;
         long i;
         symbol_info symbolinfo ;
 
         storage_needed = bfd_get_symtab_upper_bound(ibfd);
 
 
         symbol_table =  (void *)(unsigned long)xmalloc(storage_needed);
         number_of_symbols =  bfd_canonicalize_symtab (ibfd, symbol_table);
 
 
        printf("Scanning %i symbols\n", number_of_symbols);
        for(i=0;i<number_of_symbols;i++)
        {
                if (symbol_table[i]->section==NULL) continue;
                
                bfd_symbol_info(symbol_table[i],&symbolinfo);
                if (strcmp(name, symbolinfo.name))  continue;
 
                printf("Section %s  ",symbol_table[i]->section->name);
                printf("Symbol \"%s\"  value 0x%x\n",
                         symbolinfo.name, symbolinfo.value);
        }
 
 
}
 
int main()
{
    char our_self_path[1024];
    bfd *ibfd;
    char **matching;
 
    asection *psection;
 
    bfd_init();
 
    get_self_path(our_self_path, sizeof(our_self_path));
    printf("our elf file path:%s\n", our_self_path);
 
    ibfd = bfd_openr(our_self_path, NULL);
    bfd_check_format_matches(ibfd, bfd_object, &matching);
 
    printf("number of sections = %d\n", bfd_count_sections(ibfd));
 
    /* 遍历所有section，让section_proc对每一个section进行处理 */
    bfd_map_over_sections(ibfd,  section_proc,  NULL);
 
    /* 查找特定名称的section，打印出其信息 */
    psection = bfd_get_section_by_name(ibfd, "my_test_sec");
    printf("section name=%s; start_address=0x%llx; size=%d\n"
         , psection->name
         , psection->vma
         , psection->size);
 
    /* 打印出my_test_sec section中的3个uint64_t变量的值 */
    {
        uint64_t *pu64 = (void *) psection->vma;
        printf("%lu %lu %lu \n", pu64[0], pu64[1], pu64[2]);
    }
 
    printf("address of a_haha=%p\n", &a_haha);
 
    /* 遍历所有符号，以找出名称为a_haha的符号 */
    search_a_given_symbol(ibfd, "a_haha");
    return 0;
}
