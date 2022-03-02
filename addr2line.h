#ifndef _ADDR2LINE_H_
#define _ADDR2LINE_H_


#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

static bfd *abfd;
static asection *section;
static asymbol **syms; /* Symbol table.  */

static void slurp_symtab();
static void find_address_in_section(bfd *, asection *, void *);
static void find_offset_in_section(bfd *, asection *);
static void bfd_nonfatal(const char *string);
static int bfd_fatal(const char *string);
static void list_matching_formats(char **p);


int file_bfd_open(const char *file_name, const char *section_name);
void file_bfd_close();
unsigned int translate_address(const char* addr);
const char* get_file_name();
const char* get_function_name();
unsigned int get_line();
unsigned int get_discriminator();
int get_this_file_name(char* file_name);

static bfd_vma pc;
static const char *filename;
static const char *functionname;
static unsigned int line;
static unsigned int discriminator;
static bool found;

const enum add2line_status{ok,error};
const enum translate_address_result{no_found=0x10,nonsense=0x100,file_flag=0x1000,function_flag=0x10000,line_flag=0x100000,discriminator_flag=0x1000000};

#endif