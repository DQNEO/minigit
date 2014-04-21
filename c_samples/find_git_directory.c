/**
 * cwdから親へたどっていって.gitディレクトリを探す
 */
#include <stdio.h>
#include <string.h>

#define PATH_MAX 1024
#define X_OK 0

/**
 * @return bool
 */
int is_git_directory(const char *suspect)
{
  char path_to_gitdir[PATH_MAX + 1];
  strcpy(path_to_gitdir, suspect);
  strcpy(path_to_gitdir + strlen(suspect), "/.git");
  if (access(path_to_gitdir, X_OK)) {
    return 0; // false
  }

  return 1; // true
}

int main() {
  char cwd[PATH_MAX + 1];

  int offset;

  if (!getcwd(cwd, sizeof(cwd) - 1)) {
    fprintf(stderr, "Unable to read cwd");
    return 1;
  }
  offset = strlen(cwd);

  while (offset > 1) {
    if (is_git_directory(cwd)) {
      printf(".git found: %s/.git\n", cwd);
      return 0;
    }

    while (offset-- && cwd[offset] != '/') ;
    cwd[offset] = '\0';
  }

  return 0;
}
