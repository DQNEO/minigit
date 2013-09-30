#include <stdio.h>

/**
 * test if a string is a substring of another;
 *
 */
int is_substr(const char *s,const char *l)
{
  while (*s) {
    if (*(s++) != *(l++)) {
      return 0;
    }
  }

  return 1;
}


int main()
{
  char s[] = "qwerty";
  char l[]  = "qwertyuiop";

  printf("%d\n", is_substr(s, l));
  return 0;
}

