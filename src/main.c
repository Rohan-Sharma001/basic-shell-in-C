#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int default_return = -1;
char *commands[] = {"exit", "echo", "type"};
int invalid_command(char *inputt);
int echo(char *inputt);
int exitt(char *inputt);
int typef(char *inputt);
int (*func[]) (char *) = {exitt, echo, typef};



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
      invalid_command(input);
    }
    else if (ret != default_return) {
      return ret;
    }
  }
  return 0;
}

int invalid_command(char *inputt) {
  printf("%s: command not found\n", inputt);
  return default_return;
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

