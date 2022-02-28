#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>

/* Flags passed to the name demangler.  */
// static int demangle_flags = DMGL_PARAMS | DMGL_ANSI;

static int naddr;      /* Number of addresses to process.  */
static char **addr;    /* Hex addresses to process.  */
static asymbol **syms; /* Symbol table.  */
static char *program_name;

static void slurp_symtab(bfd *);
static void find_address_in_section(bfd *, asection *, void *);
static void find_offset_in_section(bfd *, asection *);
static void translate_addresses(bfd *, asection *);

static void bfd_nonfatal(const char *string);
static void bfd_fatal(const char *string);
static void list_matching_formats(char **p);


static void list_matching_formats(char **p)
{
  fflush(stdout);
  fprintf(stderr, "%s: Matching formats:", program_name);
  while (*p)
    fprintf(stderr, " %s", *p++);
  fputc('\n', stderr);
}

static void bfd_nonfatal(const char *string)
{
  const char *errmsg;
  enum bfd_error err = bfd_get_error();

  if (err == bfd_error_no_error)
    errmsg = "cause of error unknown";
  else
    errmsg = bfd_errmsg(err);
  fflush(stdout);
  if (string)
    fprintf(stderr, "%s: %s: %s\n", program_name, string, errmsg);
  else
    fprintf(stderr, "%s: %s\n", program_name, errmsg);
}

static void bfd_fatal(const char *string)
{
  bfd_nonfatal(string);
  exit(1);
}

/* Read in the symbol table.  */

static void
slurp_symtab(bfd *abfd)
{
  long storage;
  long symcount;
  bool dynamic = false;

  if ((bfd_get_file_flags(abfd) & HAS_SYMS) == 0)
    return;

  storage = bfd_get_symtab_upper_bound(abfd);
  if (storage == 0)
  {
    storage = bfd_get_dynamic_symtab_upper_bound(abfd);
    dynamic = true;
  }
  if (storage < 0)
    bfd_fatal(bfd_get_filename(abfd));

  syms = (asymbol **)malloc(storage);
  if (dynamic)
    symcount = bfd_canonicalize_dynamic_symtab(abfd, syms);
  else
    symcount = bfd_canonicalize_symtab(abfd, syms);
  if (symcount < 0)
    bfd_fatal(bfd_get_filename(abfd));

  /* If there are no symbols left after canonicalization and
   *      we have not tried the dynamic symbols then give them a go.  */
  if (symcount == 0 && !dynamic && (storage = bfd_get_dynamic_symtab_upper_bound(abfd)) > 0)
  {
    free(syms);
    syms = malloc(storage);
    symcount = bfd_canonicalize_dynamic_symtab(abfd, syms);
  }

  /* PR 17512: file: 2a1d3b5b.
   *      Do not pretend that we have some symbols when we don't.  */
  if (symcount <= 0)
  {
    free(syms);
    syms = NULL;
  }
}

/* These global variables are used to pass information between
 *    translate_addresses and find_address_in_section.  */

static bfd_vma pc;
static const char *filename;
static const char *functionname;
static unsigned int line;
static unsigned int discriminator;
static bool found;

/* Look for an address in a section.  This is called via
 *    bfd_map_over_sections.  */

static void
find_address_in_section(bfd *abfd, asection *section,
                        void *data ATTRIBUTE_UNUSED)
{
  bfd_vma vma;
  bfd_size_type size;

  if (found)
    return;

  if ((bfd_section_flags(section) & SEC_ALLOC) == 0)
    return;

  vma = bfd_section_vma(section);
  if (pc < vma)
    return;

  size = bfd_section_size(section);
  if (pc >= vma + size)
    return;

  found = bfd_find_nearest_line_discriminator(abfd, section, syms, pc - vma,
                                              &filename, &functionname,
                                              &line, &discriminator);
}

/* Look for an offset in a section.  This is directly called.  */

static void
find_offset_in_section(bfd *abfd, asection *section)
{
  bfd_size_type size;

  if (found)
    return;

  if ((bfd_section_flags(section) & SEC_ALLOC) == 0)
    return;

  size = bfd_section_size(section);
  if (pc >= size)
    return;

  found = bfd_find_nearest_line_discriminator(abfd, section, syms, pc,
                                              &filename, &functionname,
                                              &line, &discriminator);
}

/* Read hexadecimal addresses from stdin, translate into
 *    file_name:line_number and optionally function name.  */

static void
translate_addresses(bfd *abfd, asection *section)
{
  int read_stdin = (naddr == 0);

  for (;;)
  {
    if (read_stdin)
    {
      char addr_hex[100];

      if (fgets(addr_hex, sizeof addr_hex, stdin) == NULL)
        break;
      pc = bfd_scan_vma(addr_hex, NULL, 16);
    }
    else
    {
      if (naddr <= 0)
        break;
      --naddr;
      pc = bfd_scan_vma(*addr++, NULL, 16);
    }

    // for(int i=0;i<naddr;i++)
    // {
    //   free(addr[i]);
    // }
    // free(addr);
    // 依赖elf-bfd，难以解决，也不知道作用，先注释掉。
    // if (bfd_get_flavour(abfd) == bfd_target_elf_flavour)
    // {
    //   const struct elf_backend_data *bed = get_elf_backend_data(abfd);
    //   bfd_vma sign = (bfd_vma)1 << (bed->s->arch_size - 1);

    //   pc &= (sign << 1) - 1;
    //   if (bed->sign_extend_vma)
    //     pc = (pc ^ sign) - sign;
    // }

    found = false;
    if (section)
      find_offset_in_section(abfd, section);
    else
      bfd_map_over_sections(abfd, find_address_in_section, NULL);

    if (!found)
    {
      printf("??:0\n");
    }
    else
    {
      while (1)
      {
        printf("%s:", filename ? filename : "??");
        if (line != 0)
        {
          if (discriminator != 0)
            printf("%u (discriminator %u)\n", line, discriminator);
          else
            printf("%u\n", line);
        }
        else
          printf("?\n");
        // if (!unwind_inlines)
        found = false;
        // else
        //   found = bfd_find_inliner_info(abfd, &filename, &functionname,
        //                                 &line);
        if (!found)
          break;
        //         if (pretty_print)
        //           /* Note for translators: This printf is used to join the
        //  *              line number/file name pair that has just been printed with
        //  *                           the line number/file name pair that is going to be printed
        //  *                                        by the next iteration of the while loop.  Eg:
        //  *
        //  *                                                       123:bar.c (inlined by) 456:main.c  */
        //           printf(_(" (inlined by) "));
      }
    }

    /* fflush() is essential for using this command as a server
     *        child process that reads addresses from a pipe and responds
     *               with line number information, processing one address at a
     *                      time.  */
    fflush(stdout);
  }
}

/* Process a file.  Returns an exit value for main().  */

static int
process_file(const char *file_name, const char *section_name,
             const char *target)
{
  bfd *abfd;
  asection *section;
  char **matching;


  abfd = bfd_openr(file_name, target);
  if (abfd == NULL)
    bfd_fatal(file_name);
  
  /* Decompress sections.  */
  abfd->flags |= BFD_DECOMPRESS;

  if (bfd_check_format(abfd, bfd_archive))
    printf("%s: cannot get addresses from archive", file_name);

  if (!bfd_check_format_matches(abfd, bfd_object, &matching))
  {
    bfd_nonfatal(bfd_get_filename(abfd));
    if (bfd_get_error() == bfd_error_file_ambiguously_recognized)
    {
      list_matching_formats(matching);
      free(matching);
    }
    exit(1);
  }

  if (section_name != NULL)
  {
    section = bfd_get_section_by_name(abfd, section_name);
    if (section == NULL)
      printf(("%s: cannot find section %s"), file_name, section_name);
  }
  else
    section = NULL;

  slurp_symtab(abfd);

  translate_addresses(abfd, section);

  free(syms);
  syms = NULL;

  bfd_close(abfd);

  return 0;
}

int main(int argc, char **argv)
{
  const char *file_name;
  const char *section_name;
  char *target;

#ifdef HAVE_LC_MESSAGES
  setlocale(LC_MESSAGES, "");
#endif
  // setlocale(LC_CTYPE, "");
  // bindtextdomain(PACKAGE, LOCALEDIR);
  // textdomain(PACKAGE);

  // program_name = *argv;
  // xmalloc_set_program_name(program_name);
  // bfd_set_error_program_name(program_name);

  // expandargv(&argc, &argv);

  // if (bfd_init() != BFD_INIT_MAGIC)
  //   printf("fatal error: libbfd ABI mismatch");
  // set_default_bfd_target();

  file_name = "./bt";
  section_name = NULL;
  target = NULL;

  if (file_name == NULL)
    file_name = "a.out";
  addr = (char **)malloc(sizeof(char *));
  addr[0] = (char *)malloc(sizeof(char) * 8);
  addr[0] = "0400837";
  naddr = 1;

  return process_file(file_name, section_name, target);
}
