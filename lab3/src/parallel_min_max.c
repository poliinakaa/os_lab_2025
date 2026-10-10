#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

struct Channel {
  int read_fd;
  char filename[40];
};

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            break;
          case 1:
            array_size = atoi(optarg);
            break;
          case 2:
            pnum = atoi(optarg);
            break;
          default:
            fprintf(stderr, "Unknown option\n");
            return 1;
        }
        break;
      case 'f':
        with_files = true;
        break;
      default:
        fprintf(stderr, "Invalid command line option\n");
        return 1;
    }
  }

  if (optind < argc || seed <= 0 || array_size <= 0 || pnum <= 0) {
    fprintf(stderr,
            "Usage: %s --seed NUM --array_size NUM --pnum NUM [--by_files]\n",
            argv[0]);
    return 1;
  }

  int *array = malloc((size_t)array_size * sizeof(int));
  struct Channel *channels = calloc((size_t)pnum, sizeof(*channels));
  pid_t *child_pids = calloc((size_t)pnum, sizeof(*child_pids));
  if (array == NULL || channels == NULL || child_pids == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    free(array);
    free(channels);
    free(child_pids);
    return 1;
  }
  GenerateArray(array, (unsigned int)array_size, (unsigned int)seed);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int created = 0;
  bool error = false;

  for (int i = 0; i < pnum; i++) {
    int fds[2] = {-1, -1};
    if (with_files) {
      strcpy(channels[i].filename, "/tmp/lab3_minmax_XXXXXX");
      int fd = mkstemp(channels[i].filename);
      if (fd == -1) {
        perror("mkstemp");
        channels[i].filename[0] = '\0';
        error = true;
        break;
      }
      close(fd);
    } else if (pipe(fds) == -1) {
      perror("pipe");
      error = true;
      break;
    }

    pid_t child_pid = fork();
    if (child_pid == -1) {
      perror("fork");
      if (fds[0] != -1) close(fds[0]);
      if (fds[1] != -1) close(fds[1]);
      error = true;
      break;
    }

    if (child_pid == 0) {
      if (!with_files) {
        close(fds[0]);
        for (int j = 0; j < i; j++) close(channels[j].read_fd);
      }

      unsigned int begin = (unsigned int)((long long)i * array_size / pnum);
      unsigned int end = (unsigned int)((long long)(i + 1) * array_size / pnum);
      struct MinMax local = GetMinMax(array, begin, end);

      if (with_files) {
        FILE *file = fopen(channels[i].filename, "w");
        if (file == NULL) {
          perror("fopen");
          _exit(1);
        }
        int write_failed = fprintf(file, "%d %d\n", local.min, local.max) < 0;
        if (fclose(file) != 0 || write_failed) _exit(1);
      } else {
        if (write(fds[1], &local, sizeof(local)) != (ssize_t)sizeof(local)) {
          perror("write");
          _exit(1);
        }
        close(fds[1]);
      }
      _exit(0);
    }

    child_pids[i] = child_pid;
    created++;
    if (!with_files) {
      close(fds[1]);
      channels[i].read_fd = fds[0];
    }
  }

  for (int i = 0; i < created; i++) {
    int status;
    if (waitpid(child_pids[i], &status, 0) == -1 ||
        !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      fprintf(stderr, "Child process failed\n");
      error = true;
    }
  }

  struct MinMax min_max = {INT_MAX, INT_MIN};
  if (!error) {
    for (int i = 0; i < created; i++) {
      struct MinMax local;
      if (with_files) {
        FILE *file = fopen(channels[i].filename, "r");
        if (file == NULL || fscanf(file, "%d %d", &local.min, &local.max) != 2) {
          fprintf(stderr, "Cannot read result file\n");
          if (file != NULL) fclose(file);
          error = true;
          break;
        }
        fclose(file);
      } else {
        if (read(channels[i].read_fd, &local, sizeof(local)) !=
            (ssize_t)sizeof(local)) {
          fprintf(stderr, "Cannot read pipe\n");
          error = true;
          break;
        }
      }
      if (local.min < min_max.min) min_max.min = local.min;
      if (local.max > min_max.max) min_max.max = local.max;
    }
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  for (int i = 0; i < pnum; i++) {
    if (with_files) {
      if (channels[i].filename[0] != '\0') unlink(channels[i].filename);
    } else if (i < created) {
      close(channels[i].read_fd);
    }
  }
  free(array);
  free(channels);
  free(child_pids);

  if (error) return 1;
  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  return 0;
}
