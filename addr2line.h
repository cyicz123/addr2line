#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>

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
int traslate_address(const char* addr);
char* get_file_name();
char* get_function_name();
unsigned int get_line();
unsigned int get_discriminator();

static bfd_vma pc;
static const char *filename;
static const char *functionname;
static unsigned int line;
static unsigned int discriminator;
static bool found;

enum add2line_status{ok,error};
enum translate_address_result{no_found=2,null_filename,zero_line,zero_discriminator};