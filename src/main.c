#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

const int maxBuff = 1024;

int default_return = -1;
int default_fail = -2;
char *commands[] = {"exit", "echo", "type", "pwd", "cd"};
int invalid_command(char **inputt, char *buff);
int echo(char **inputt, char *buff);
int exitt(char **inputt, char *buff);
int typef(char **inputt, char *buff);
int runExecutable(char **inputt, char *buff);
int pwd(char **inputt, char *buff);
int cd(char **inputt, char *cdto);
char **argSeparate(char *S);
int (*func[]) (char **, char *) = {exitt, echo, typef, pwd, cd};



int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);

  char input[maxBuff];
  while (1) { 
    printf("$ ");
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';

    char **argss = argSeparate(input);
    char buff[maxBuff]; memset(buff, '\0', maxBuff);
    char *space = " ";
    for (int i = 0; i < maxBuff, argss[i] != NULL; i++) {
      strcat(buff, argss[i]);
      strcat(buff, space);
    }
    buff[strlen(buff) - 1] = '\0';
    int returnVal, executed = 0;
    for (int i = 0; i < sizeof(func)/sizeof(func[0]); i++) {
      if (!strcmp(argss[0], commands[i])) {returnVal = func[i](argss, buff); executed = 1; break;}
    }
    if (!executed) {
      returnVal = runExecutable(argss, buff);
    }
    for (int i = 0; i < maxBuff, argss[i] != NULL; i++) {
      free(argss[i]);
    }
    free(argss);
    if (returnVal != default_fail && returnVal != default_return) {
      return returnVal;
    }
  }
}


/*
  ARGUMENT SEPARATOR
*/
char **argSeparate(char *S) {
  char **arr = (char**)calloc(maxBuff, sizeof(char));
  char buff[maxBuff];
  int t = 0;
  for (int i = 0; i < strlen(S); i++) {
      memset(buff, '\0', sizeof(buff));
      buff[0] = '\0';
      if (S[i] == ' ') continue;
      int inquotes = 0;
      int inDquotes = 0;
      int j;
      for (j = i; j < strlen(S); j++) {
        if (S[j] == '\'' && !(inDquotes)) {inquotes ^= 1;}
        else if (S[j] == '\"' && !(inquotes)) {inDquotes ^= 1;}
        else if (S[j] == '\\' && !(inquotes) && !(inDquotes) && j < strlen(S)-1) {j++;}
        else if (S[j] == ' ' && !inquotes && !inDquotes) {
            break;
        }
      }
      inquotes = 0, inDquotes = 0;
      for (int k = i; k < j; k++) {
          if (S[k] == '\'' && !(inDquotes)) {inquotes ^= 1;}
          else if (S[k] == '\"' && !(inquotes)) {inDquotes ^= 1;}
          else if (S[k] == '\\' && !(inquotes) && !(inDquotes) && k < j-1) {k++; buff[strlen(buff)] = S[k];}
          else if (S[k] == '\\' && (S[k+1] == '\\' || S[k] == '$' || S[k+1] == '\"' || S[k+1] == '\n') && inDquotes) {k++;buff[strlen(buff)] = S[k];}
          else buff[strlen(buff)] = S[k];
      }
    //printf("%s\n", buff);
      arr[t] = strdup(buff);
      i = j;
      t++;
  }
  return arr;
}
/*
  SHELL BUILTIN COMMANDS
*/

int cd(char **inputt, char *cdto) {
  char homeDir[1024];
  strcpy(homeDir, getenv("HOME"));
  char *changeto;
  if (cdto[3] == '~') {
    strcat(homeDir, cdto+4);
    changeto = homeDir;
  }
  else {changeto = cdto+3;}
  int status = chdir(changeto);
  if (!status) {
    return default_return;
  } 
  else {
    printf("cd: %s: No such file or directory\n", changeto);
    return default_fail;
  }
}

int pwd(char **inputt, char *buff) {
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  printf("%s\n", cwd);
  return default_return;
}

int invalid_command(char **inputt, char *buff) {
  printf("%s: command not found\n", buff);
  return default_return;
}

int runExecutable(char **executer, char *buff) {

  //strcat(exec, executer[0]);
  int output = -1;
  //PATH LOOP
  char *pathVar = strdup(getenv("PATH"));
  char *token = strtok(pathVar, ":");
  int foundSomething = 0;
  
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
    foundSomething = foundSomething | found;
    if (found) break;
  }
  if (!foundSomething) {
    invalid_command(executer, buff);
  }
  return retVal;
}

int typef(char **inputt, char *buff) {

  for (int i = 1; i < maxBuff, inputt[i] != NULL; i++) {
    char *functionName = inputt[i];
    int found = 0;
    for (int i = 0; i < sizeof(func)/sizeof(func[0]); i++) {
      if (!strcmp(functionName, commands[i])) {printf("%s is a shell builtin\n", commands[i]); found = 1; break;}
    }
    //check in path if not a builtin
    if (!found) {
      char *pathVar = strdup(getenv("PATH"));
      
      char *token = strtok(pathVar, ":");

      while (token != NULL) {
        char *fullPath = malloc(strlen(token) + 1 + strlen(functionName) + 1);
        sprintf(fullPath, "%s/%s", token, functionName);

        if (!access(fullPath, F_OK)) {
          printf("%s is %s\n", inputt[i], fullPath);
          found = 1;
          free(fullPath);
          break;
        }
        free(fullPath);
        token = strtok(NULL, ":");
      }
    }
    if (!found) {
      printf("%s: not found\n", functionName);
    }
  }
  return default_return;
}

int exitt(char **inputt, char *buff) {
  if (inputt[1] != NULL) {
    return atoi(inputt[1]);
  }
  return 0;
}

int echo(char **inputt, char *buff) {
  printf("%s\n", buff+5);
  return default_return;
}