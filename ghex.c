#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_WIDTH 256
#define DEFAULT_WIDTH 16
#define MAX_PATTERN_LEN 256

static int parse_hex_string(const char *hex_str, unsigned char *out, size_t *out_len)
{
  size_t count = 0;
  const char *p = hex_str;

  while (*p && count < MAX_PATTERN_LEN)
  {
    while (isspace((unsigned char)*p))
      p++;
    if (!*p)
      break;

    if (isxdigit((unsigned char)p[0]) && isxdigit((unsigned char)p[1]))
    {
      int high = tolower((unsigned char)p[0]);
      int low = tolower((unsigned char)p[1]);
      high = (high >= '0' && high <= '9') ? high - '0' : high - 'a' + 10;
      low = (low >= '0' && low <= '9') ? low - '0' : low - 'a' + 10;
      out[count++] = (unsigned char)((high << 4) | low);
      p += 2;
    }
    else
    {
      return 0; /* Invalid hex pair */
    }
  }

  *out_len = count;
  return count > 0;
}

static void dump_hex_block(FILE *fp, long start, long length, int width, int no_ascii)
{
  unsigned char line[MAX_WIDTH];
  long fx = start;
  long bytes_to_read = length;
  int x = 0, i;
  int c;

  fseek(fp, start, SEEK_SET);

  while (bytes_to_read > 0 && (c = fgetc(fp)) != EOF)
  {
    if (x == 0)
    {
      printf("%08lX: ", fx);
    }

    line[x] = (unsigned char)c;
    printf("%02X ", line[x]);
    x++;
    fx++;
    bytes_to_read--;

    if (x == width)
    {
      if (!no_ascii)
      {
        printf(" |");
        for (i = 0; i < width; i++)
        {
          putchar(isprint(line[i]) ? line[i] : '.');
        }
        printf("|");
      }
      printf("\n");
      x = 0;
    }
  }

  if (x > 0)
  {
    if (!no_ascii)
    {
      for (i = x; i < width; i++)
      {
        printf("   ");
      }
      printf(" |");
      for (i = 0; i < x; i++)
      {
        putchar(isprint(line[i]) ? line[i] : '.');
      }
      printf("|");
    }
    printf("\n");
  }
}

int main(int argc, char *argv[])
{
  // c is the current character being read from the file
  // fx is global byte offset
  // x is current column index
  // default columns to be shown is 16
  int argx,
      c,
      fx,
      x,
      i,
      width = DEFAULT_WIDTH,
      no_ascii = 0;
  // store each buffer byte to print later in ASCII
  long start_offset = 0,
       end_offset = -1,
       rows_limit = -1,
       rows_printed = 0,
       temp_val;
  unsigned char line[MAX_WIDTH];
  unsigned char search_pattern[MAX_PATTERN_LEN];
  size_t search_len = 0;
  int search_mode = 0;

  FILE *fp;

  /*printf("argc=%i\n",argc);*/

  if (argc == 1)
  {
    printf("Ghex, a simple Hex-dump tool.\n");
    printf("Enter filenames as command line arguments such as:\n");
    printf("%s --width=16 --start=0 --end=64 --no-ascii file(s).ext\n", argv[0]);
    printf("%s --search-text=\"gcc\" file(s).ext\n", argv[0]);
    printf("%s --search-hex=\"FF\" --no-ascii file(s).ext\n", argv[0]);
  }

  argx = 1;

  while (argx < argc)
  {
    if (sscanf(argv[argx], "--width=%i", &x))
    {
      width = x;
    }
    else if (sscanf(argv[argx], "--start=%li", &temp_val))
    {
      start_offset = temp_val;
    }
    else if (sscanf(argv[argx], "--end=%li", &temp_val))
    {
      end_offset = temp_val;
    }
    else if (sscanf(argv[argx], "--rows=%li", &temp_val))
    {
      rows_limit = temp_val;
    }
    else if (strcmp(argv[argx], "--no-ascii") == 0)
    {
      no_ascii = 1;
    }
    else if (strncmp(argv[argx], "--search-text=", 14) == 0)
    {
      const char *pattern_str = argv[argx] + 14;
      search_len = strlen(pattern_str);
      if (search_len > MAX_PATTERN_LEN)
        search_len = MAX_PATTERN_LEN;

      memcpy(search_pattern, pattern_str, search_len);
      search_mode = (search_len > 0);
    }
    else if (strncmp(argv[argx], "--search-hex=", 13) == 0)
    {
      const char *pattern_str = argv[argx] + 13;

      if (parse_hex_string(pattern_str, search_pattern, &search_len))
      {
        search_mode = 1;
      }
      else
      {
        printf("Error: Invalid hex pattern '%s'\n", pattern_str);
      }
    }
    else
    {
      fp = fopen(argv[argx], "rb");
      if (!fp)
      {
        printf("Error: Cannot open file %s: ", argv[argx]);
        printf("No such file or directory\n");
      }
      else
      {

        if (search_mode)
        {
          // search hex/string mode
          long match_count = 0;
          size_t matched_bytes = 0;
          long file_pos = start_offset;

          if (start_offset > 0)
          {
            fseek(fp, start_offset, SEEK_SET);
          }

          while ((end_offset == -1 || file_pos <= end_offset) && (c = fgetc(fp)) != EOF)
          {
            if ((unsigned char)c == search_pattern[matched_bytes])
            {
              matched_bytes++;
              if (matched_bytes == search_len)
              {
                long match_offset = file_pos - (search_len - 1);
                printf("\n[Match #%ld at offset 0x%08lX (%ld)]\n", ++match_count, match_offset, match_offset);

                /* Dump matching row */
                long dump_rows = (rows_limit > 0) ? rows_limit : 1;
                dump_hex_block(fp, match_offset, dump_rows * width, width, no_ascii);

                matched_bytes = 0;
                fseek(fp, match_offset + 1, SEEK_SET);
                file_pos = match_offset;
              }
            }
            else
            {
              if (matched_bytes > 0)
              {
                fseek(fp, file_pos - matched_bytes + 1, SEEK_SET);
                file_pos = file_pos - matched_bytes;
                matched_bytes = 0;
              }
            }
            file_pos++;
          }
        }
        else
        {
          // standard dump hex mode
        }
        // printf("Reading bytes of file %s:\n",argv[argx]);

        if (start_offset > 0)
        {
          fseek(fp, start_offset, SEEK_SET);
          fx = start_offset;
        }
        else
        {
          fx = 0;
        }

        rows_printed = 0;
        x = 0;
        while (1)
        {
          if (rows_limit >= 0 && rows_printed >= rows_limit)
          {
            break;
          }
          if (end_offset >= 0 && fx > end_offset)
          {
            break;
          }

          c = fgetc(fp);
          if (c == EOF)
          {
            break;
          }

          if (x == 0)
          {
            printf("%08X: ", fx);
          }

          line[x] = (unsigned char)c;
          printf("%02X ", line[x]);
          x++;

          if (x == width)
          {
            if (!no_ascii)
            {
              printf(" |");
              for (i = 0; i < width; i++)
              {
                putchar(isprint(line[i]) ? line[i] : '.');
              }
              printf("|");
            }
            printf("\n");
            rows_printed++;
            x = 0;
          }

          fx++;
        }

        if (x > 0 && (rows_limit < 0 || rows_printed < rows_limit))
        {
          if (!no_ascii)
          {
            for (i = x; i < width; i++)
            {
              printf("   ");
            }
            printf(" |");
            for (i = 0; i < x; i++)
            {
              putchar(isprint(line[i]) ? line[i] : '.');
            }
            printf("|");
          }
          printf("\n");
        }

        fclose(fp);
        printf("\n");
      }
      // reset parameters for next file
      start_offset = 0;
      end_offset = -1;
      rows_limit = -1;
      width = DEFAULT_WIDTH;
      no_ascii = 0;
      search_mode = 0;
      search_len = 0;
    }

    argx++;
  }

  return 0;
}
