#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // Uncomment this block to pass the first stage
 
  // Wait for user input
  char input[100];
  while (1) { 
    printf("$ ");
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';
    
    //exit
    char *exit_str = "exit 0";
    char *echo_str = "echo";
    if (!strcmp(input, exit_str)) {
      return 0;
    }
    else if (!strcmp(input, echo_str)) {
      printf("%s", s[5]);
    }
    else {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
