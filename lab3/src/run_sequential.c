#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s seed arraysize\n", argv[0]);
    return 1;
  }

  pid_t child_pid = fork();
  if (child_pid == -1) {
    perror("fork");
    return 1;
  }

  if (child_pid == 0) {
    execl("./sequential_min_max", "sequential_min_max", argv[1], argv[2],
          (char *)NULL);
    perror("execl");
    _exit(1);
  }

  int status;
  if (waitpid(child_pid, &status, 0) == -1) {
    perror("waitpid");
    return 1;
  }
  return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
