#include "payload.h"
#include "../defines.h"
#include "../trace.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int depcheck(int id) {
  int result;
  switch (id) {
  case DEP_GCC:
    result = system("which gcc >/dev/null 2>&1");
    if (result == 0)
      return 0;
    else
      return 1;
  case DEP_PKGCONFIG:
    result = system("which pkg-config >/dev/null 2>&1");
    if (result == 0)
      return 0;
    else
      return 1;
  case DEP_GTK4:
    result = system("pkg-config --exists gtk4 >/dev/null 2>&1");
    if (result == 0)
      return 0;
    else
      return 1;
  case DEP_GTK4_LAYERSHELL:
    result = system("pkg-config --exists gtk4-layer-shell-0 >/dev/null 2>&1");
    if (result == 0)
      return 0;
    else
      return 1;
  case DEP_GIT:
    result = system("which git >/dev/null 2>&1");
    if (result == 0)
      return 0;
    else
      return 1;
  }
  return -1;
}

int clonegit(void) {
  int result;
  char clone_cmd[512];
  
  system("rm -rf /tmp/degrli-src"); // prevents error 128

  snprintf(clone_cmd, sizeof(clone_cmd),
           "git clone https://github.com/potato-master369/desktop-gremlin-linux /tmp/degrli-src >/dev/null 2>&1");
  result = system(clone_cmd);
  if (result != 0) {
    return result;
  }

}

int cd_src(void) {
  char checkout_cmd[512];
  const char *src_dir = "/tmp/degrli-src";

  snprintf(checkout_cmd, sizeof(checkout_cmd),
           "git checkout %s >/dev/null 2>&1",
           DEGRLI_CHECKOUT_VERSION);
  
  int result = chdir(src_dir);
  if (result != 0) {
    return result;
  }
  return system(checkout_cmd);
}

int build_src(void) {
  int result;
  result = system("make all");
  return result;
}

int install_src(void) {
  pid_t pid = fork();

  if (pid < 0) {
    // Fork failed
    trace_log(ERROR, "Fork failed: %s\n", strerror(errno));
    return 1;
  }

  if (pid == 0) {
    // --- Child Process ---
    // Explicitly change to source directory before running make
    char *args[] = {"pkexec", "sh", "-c", "cd /tmp/degrli-src && make install", NULL};

    // execvp looks for pkexec in the system PATH
    execvp(args[0], args);

    // execvp only returns if an error occurred
    trace_log(ERROR, "Failed to execute pkexec: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  } else {
    // --- Parent Process ---
    int status;

    // Wait for the child process (pkexec) to finish
    if (waitpid(pid, &status, 0) == -1) {
      trace_log(ERROR, "waitpid failed: %s\n", strerror(errno));
      return 1;
    }

    // Check how the child process terminated
    if (WIFEXITED(status)) {
      int exit_code = WEXITSTATUS(status);
      if (exit_code == 0) {
        trace_log(INFO, "Installation completed successfully!\n");
        return 0;
      } else if (exit_code == 127) {
        trace_log(ERROR, "Error: pkexec or make command not found.\n");
        return 1;
      } else if (exit_code == 126) {
        trace_log(ERROR, "Error: Authentication was cancelled or failed.\n");
        return 1;
      } else {
        trace_log(ERROR, "Installation failed with exit code: %d\n", exit_code);
        return 1;
      }
    } else if (WIFSIGNALED(status)) {
      trace_log(ERROR, "Process was killed by signal %d\n", WTERMSIG(status));
      return 1;
    }
  }

  return 0;
}
