#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int default_return = -1;
int default_fail = -2;
char *commands[] = {"exit", "echo", "type", "pwd", "cd"};
int invalid_command(char *inputt);
int echo(char *inputt);
int exitt(char *inputt);
int typef(char *inputt);
int runExecutable(char *inputt);
int cd(char *inputt);
int pwd(char *input);
int (*func[]) (char *) = {exitt, echo, typef, pwd, cd};



int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);

  char input[100];
  while (1) { 
    printf("$ ");
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';

    int executed = 0;
    int ret;
    for (int i = 0; i < sizeof(func)/sizeof(func[0]); i++) {
      if (!strncmp(input, commands[i], strlen(commands[i]))) {ret = func[i](input); executed = 1; break;}
    }
    if (!executed) {
      ret = runExecutable(input);
      if (ret == default_return) {executed = 1;}
    }
    if (!executed) {
      invalid_command(input);
    }
    else if (ret != default_return && ret != default_fail) {
      return ret;
    }
  }
  return 0;
}



/*
  SHELL BUILTIN COMMANDS
*/
int cd(char *inputt) {
  int status = chdir(inputt + 3);
  if (!status) {
    return default_return;
  } 
  else {
    printf("cd: %s: No such file or directory\n", inputt+3);
    return default_fail;
  }
}

int pwd(char *inputt) {
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  printf("%s\n", cwd);
  return default_return;
}

int invalid_command(char *inputt) {
  printf("%s: command not found\n", inputt);
  return default_return;
}

int runExecutable(char *inputt) {
  int argc = 1;
  for (int i = 0; i < strlen(inputt); i++) {if (inputt[i] == ' ') argc++;}
  char **executer = (char **)malloc((argc+1)*(sizeof(char *)));

  char *inputCpy = strdup(inputt);
  char *inputToken = strtok(inputCpy, " ");

  int j = 1;
  while (j <= argc) {
    executer[j-1] = strdup(inputToken);
    inputToken = strtok(NULL, " ");
    j++;
  }
  //strcat(exec, executer[0]);
  int output = -1;
  //PATH LOOP
  char *pathVar = strdup(getenv("PATH"));
  char *token = strtok(pathVar, ":");
  
  int retVal = default_fail;
  while (token != NULL) {
    int found = 0;
    if (output != -1) break;
    char *fullPath = malloc(strlen(token) + 1 + strlen(executer[0]) + 1);
    sprintf(fullPath, "%s/%s", token, executer[0]);
    if (!access(fullPath, F_OK)) {
      found = 1;
      int pid = fork();
      if (pid == 0) {
        execv(fullPath, executer);
      }
      else if (pid > 0) {
        int status;
        waitpid(0, &status, 0);
      }
      retVal = default_return;
    }
    free(fullPath);
    token = strtok(NULL, ":");
    if (found) break;
  }
  for (int i = 0; i < argc; i++) {
    free(executer[i]);
  }

  free(executer);
  return retVal;
}

int typef(char *inputt) {
  if (strlen(inputt) > 4 && inputt[4] != ' ') {
    invalid_command(inputt);
  }
  else {
    int found = 0;
    for (int i = 0; i < sizeof(func)/sizeof(func[0]); i++) {
      if (!strcmp(inputt+5, commands[i])) {printf("%s is a shell builtin\n", commands[i]); found = 1; break;}
    }
    //check in path if not a builtin
    if (!found) {
      char *pathVar = strdup(getenv("PATH"));
      
      char *token = strtok(pathVar, ":");

      while (token != NULL) {
        char *fullPath = malloc(strlen(token) + 1 + strlen(inputt+5) + 1);
        sprintf(fullPath, "%s/%s", token, inputt+5);

        if (!access(fullPath, F_OK)) {
          printf("%s is %s\n", inputt+5, fullPath);
          found = 1;
          free(fullPath);
          break;
        }
        free(fullPath);
        token = strtok(NULL, ":");
      }
    }
    if (!found) {
      printf("%s: not found\n", inputt+5);
    }
  }
  return default_return;
}

int exitt(char *inputt) {
  char *exit_str = "exit 0";
  if (!strcmp(inputt, exit_str)) {
    return 0;
  }
  else {
    invalid_command(inputt);
  }
  return default_return;
}

int echo(char *inputt) {
  printf("%s\n", inputt+5);
  return default_return;
}

