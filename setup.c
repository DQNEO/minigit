/**
 * cwdから親へたどっていって.gitディレクトリを探す
 */
#include <stdio.h>
#include <string.h>

#define PATH_MAX 1024

int main() {
  char cwd[PATH_MAX + 1];
  char dir[PATH_MAX + 1];

  int offset;

  if (!getcwd(cwd, sizeof(cwd) - 1)) {
    fprintf(stderr, "Unable to read cwd");
    return 1;
  }
  printf("cwd = %s\n", cwd);

  offset = strlen(cwd);

  while (offset > 1) {
    while (offset-- && cwd[offset] != '/') ;
    cwd[offset] = '\0';
    printf("%s\n", cwd);
  }
  return 0;
}
