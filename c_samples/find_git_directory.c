/**
 * cwdから親へたどっていって.gitディレクトリを探す
 *
 * 元のソースでは、
 * setup.c:L643-L664 あたり
 * getcwdで取得した文字列を/区切りで順番にchdirして調査している。
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


int find_git_root_directory(char* cwd, size_t cwd_size)
{
  int offset;

  if (!getcwd(cwd, cwd_size - 1)) {
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

int main() {
  char path[PATH_MAX + 1];
  find_git_root_directory(path, sizeof(path));
  return 0;
}
