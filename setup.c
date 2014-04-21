/**
 * cwdから親へたどっていって.gitディレクトリを探す
 */
#include <stdio.h>
#include <string.h>

#define PATH_MAX 1024

int main() {
  char cwd[PATH_MAX + 1];
  char path_to_gitdir[PATH_MAX + 1];

  int offset;

  if (!getcwd(cwd, sizeof(cwd) - 1)) {
    fprintf(stderr, "Unable to read cwd");
    return 1;
  }
  offset = strlen(cwd);

  while (offset > 1) {
    strcpy(path_to_gitdir, cwd);
    strcpy(path_to_gitdir + strlen(cwd), "/.git");
    if (! access(path_to_gitdir, 0)) {
      printf(".git found: %s\n", path_to_gitdir);
      return 0;
    }

    //printf("cwd=%s\n", cwd);
    //printf("gitdir=%s\n", path_to_gitdir);

    while (offset-- && cwd[offset] != '/') ;
    cwd[offset] = '\0';
  }

  return 0;
}
