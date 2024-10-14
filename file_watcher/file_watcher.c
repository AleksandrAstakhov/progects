#include <bits/wordsize.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct Counter {
  char filename[PATH_MAX];
  int counter;
  struct Counter* next;
} Counter;

typedef struct Counters {
  struct Counter* head;
} Counters;

void increment(Counters* counters, char* filename, int value) {
  Counter* current = counters->head;
  while (current != NULL) {
    if (strncmp(current->filename, filename, PATH_MAX) == 0) {
      current->counter = value;
      return;
    }
    current = current->next;
  }
  Counter* new_head = malloc(sizeof(Counter));
  new_head->next = counters->head;
  new_head->counter = value;
  strncpy(new_head->filename, filename, PATH_MAX - 1);
  new_head->filename[PATH_MAX - 1] = '\0';
  counters->head = new_head;
}

void print(Counters* counters) {
  Counter* current = counters->head;
  while (current != NULL) {
    printf("%s:%d\n", current->filename, current->counter);
    current = current->next;
  }
}

int main(int argc, char* argv[]) {
  Counters* counters = malloc(sizeof(Counter));
  counters->head = NULL;
  pid_t pid = fork();
  if (pid == 0) {
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    execvp(argv[1], argv + 1);
  } else {
    int status;
    struct user_regs_struct state;
    char filename[PATH_MAX];
    char filepath[PATH_MAX];
    wait(&status);
    while (WIFSTOPPED(status)) {
      ptrace(PTRACE_GETREGS, pid, NULL, &state);
      ssize_t fd = state.rdi;
      ssize_t syscall = state.orig_rax;
      ssize_t byte_to_read = state.rdx;

      if (syscall == 1) {
        sprintf(filepath, "/proc/%d/fd/%ld", pid, fd);
        ssize_t filename_size;
        if ((filename_size = readlink(filepath, filename, PATH_MAX - 1)) < 0) {
          return -1;
        }
        filename[filename_size] = '\0';
        increment(counters, filename, byte_to_read);
      }
      ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
      wait(&status);
    }
  }
  ptrace(PTRACE_DETACH, pid, NULL, NULL);
  print(counters);
}
