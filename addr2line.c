#include "addr2line.h"

static void list_matching_formats(char **p)
{
  fflush(stdout);
  // fprintf(stderr, "%s: Matching formats:", program_name);
  fprintf(stderr, "Matching formats:");
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
    // fprintf(stderr, "%s: %s: %s\n", program_name, string, errmsg);
    fprintf(stderr, "%s: %s\n", string, errmsg);
  else
    // fprintf(stderr, "%s: %s\n", program_name, errmsg);
    fprintf(stderr, "%s\n", errmsg);
}

static int bfd_fatal(const char *string)
{
  bfd_nonfatal(string);
  return error;
}

/* Read in the symbol table.  */

static void slurp_symtab()
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

static void find_address_in_section(bfd *abfd, asection *section,
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

static void find_offset_in_section(bfd *abfd, asection *section)
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

int file_bfd_open(const char *file_name, const char *section_name)
{
  char **matching;

  abfd = bfd_openr(file_name, NULL);
  if (abfd == NULL)
  {
    bfd_fatal(file_name);
    return error;
  }
   

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
    return error;
  }

  if (section_name != NULL)
  {
    section = bfd_get_section_by_name(abfd, section_name);
    if (section == NULL)
      printf(("%s: cannot find section %s"), file_name, section_name);
  }
  else
    section = NULL;

  return ok;
}

void file_bfd_close()
{
  bfd_close(abfd);
}

int traslate_address(const char* addr)
{
  if(addr==NULL)
  {
    return error;
  }
  pc = bfd_scan_vma(addr,NULL,16);

  found=false;
  if (section)
    find_offset_in_section(abfd, section);
  else
    bfd_map_over_sections(abfd, find_address_in_section, NULL);

  if(!found)
  {
    return no_found;
  }
  else
  {
    if(filename==NULL)
    {
      return null_filename;
    }
    if(line==0)
    {
      return zero_line;
    }
    if(discriminator==0)
    {
      return zero_discriminator;
    }
  }
  return ok;
}

char* get_file_name()
{
  return filename;
}

char* get_function_name()
{
  return functionname;
}

unsigned int get_line()
{
  return line;
}

unsigned int get_discriminator()
{
  return discriminator;
}


int main()
{
}