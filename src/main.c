#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <sys/stat.h>

const int maxBuff = 1024;
int currentOperator;

typedef struct argStruct {
  char **command;
  int operator;
} argStruct;

char funcOutput[1024];
char funcError[1024];

int default_return = -1;
int default_fail = -2;
char *commands[] = {"exit", "echo", "type", "pwd", "cd"};
int invalid_command(char **inputt, char *buff);
int echo(char **inputt, char *buff);
int exitt(char **inputt, char *buff);
int typef(char **inputt, char *buff);
int runExecutable(char **inputt, char *buff, int prevOperator);
int pwd(char **inputt, char *buff);
int cd(char **inputt, char *cdto);
argStruct *argSeparate(char *S);
char *generatoR(const char *input, int state);
char **completerFn(const char *input, int start, int end);
int (*func[]) (char **, char *) = {exitt, echo, typef, pwd, cd};
char **matchList = commands;
int listIndex;
char *tokken;
DIR *currentDirStream = NULL;

int main(int argc, char *argv[]) {
  rl_attempted_completion_function = completerFn;
  setbuf(stdout, NULL);

  char *input;
  while (1) { 
    input = readline("$ ");
    if (input) {
      argStruct *argarr = argSeparate(input);
      char buff[maxBuff]; memset(buff, '\0', maxBuff);
      char *space = " ";
      int prevOperator = -2;
      int it = 0;

      while (prevOperator != -1) {
        if (prevOperator == 0 || prevOperator == 1 || prevOperator == 2 || prevOperator == 3) {
          //printf("%s\n", ((prevOperator)? funcOutput:funcError));
          FILE *fileptr = fopen(*(argarr[it].command), ((prevOperator <= 1)? "w": "a"));
          fprintf(fileptr, "%s", ((!prevOperator || prevOperator == 2)? funcOutput:funcError));
          fclose(fileptr);
          if (!prevOperator || prevOperator == 2) memset(funcOutput, '\0', sizeof(funcOutput));
          if (prevOperator == 1 || prevOperator == 3) memset(funcError, '\0', sizeof(funcError));
          prevOperator = argarr[it].operator;
          it++;
        }
        else {
          if (!argarr[it].command) break;
          char **argss = argarr[it].command;
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
            currentOperator = argarr[it].operator;
            returnVal = runExecutable(argss, buff, argarr[it].operator);
          }
          prevOperator = argarr[it].operator;
          for (int i = 0; i < maxBuff, argss[i] != NULL; i++) {
            free(argss[i]);
          }
          free(argss);
          if (returnVal != default_fail && returnVal != default_return) {
            free(argarr);
            free(input);
            return returnVal;
          }
          it++;
        }
      }
      if (prevOperator == -1) {
        if (funcOutput[0] || funcError[0]) printf("%s%s", funcOutput,funcError);
        memset(funcOutput, '\0', sizeof(funcOutput));
        memset(funcError, '\0', sizeof(funcError));
        sleep(1);
      }
      free(argarr);
    }
    free(input);
  }
}


/*
  ARGUMENT SEPARATOR
*/
argStruct *argSeparate(char *S) {
  argStruct *argStructArray = calloc(10, sizeof(argStruct));
  int argStructArrayIndex = 0;

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
      else if (S[j] == ' ' && !inquotes && !inDquotes) {break;}
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
    
    i = j;
    if (!strcmp(buff, ">") || !strcmp(buff, "1>") || !strcmp(buff, "2>") || !strcmp(buff, "1>>") || !strcmp(buff, ">>") || !strcmp(buff, "2>>")) {
      argStructArray[argStructArrayIndex].command = malloc(maxBuff);
      memcpy(argStructArray[argStructArrayIndex].command, arr, maxBuff);
      if (!strcmp(buff,"2>")) argStructArray[argStructArrayIndex].operator = 1;
      else if (!strcmp(buff, "1>>") || (!strcmp(buff, ">>"))) argStructArray[argStructArrayIndex].operator = 2;
      else if (!strcmp(buff, "2>>")) argStructArray[argStructArrayIndex].operator = 3;
      else argStructArray[argStructArrayIndex].operator = 0;
      argStructArrayIndex++;
      t = 0;
      memset(arr, '\0', maxBuff);
    } else {arr[t] = strdup(buff);t++;}

    if (i == strlen(S)) {
      argStructArray[argStructArrayIndex].command = malloc(maxBuff);
      memcpy(argStructArray[argStructArrayIndex].command, arr, maxBuff);
      argStructArray[argStructArrayIndex].operator = -1;
    }
  }
  free(arr);
  return argStructArray;
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
    sprintf(funcOutput, "cd: %s: No such file or directory\n", changeto);
    return default_fail;
  }
}

int pwd(char **inputt, char *buff) {
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  sprintf(funcOutput, "%s\n", cwd);
  return default_return;
}

int invalid_command(char **inputt, char *buff) {
  sprintf(funcOutput, "%s: command not found\n", buff);
  return default_return;
}

int runExecutable(char **executer, char *buff, int prevOperator) {

  //strcat(exec, executer[0]);
  char execBuff[4096]; memset(execBuff, '\0', sizeof(execBuff));
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

      //  CREATE PIPE
      int pipefd[2], pipefd2[2];
      if (pipe(pipefd) == -1 || pipe(pipefd2) == -1){
        perror("pipe");
        exit(EXIT_FAILURE);
      }

      //  FORK TO CREATE CHILD
      int pid = fork();
      if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
      }
      // HANDLE CHILD PROCESS
      else if (pid == 0) {
        close(pipefd[0]);
        close(pipefd2[0]);
        if (prevOperator == 0 || prevOperator == 2) {
          if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(EXIT_FAILURE);
          }
        }
        else if (prevOperator == 1 || prevOperator == 3) {
          if (dup2(pipefd2[1], STDERR_FILENO) == -1) {
            perror("dup2");
            _exit(EXIT_FAILURE); // Use _exit in child
          }
        }
        close(pipefd[1]);
        close(pipefd2[1]);
        execv(fullPath, executer);
        perror("execvp failed");
        _exit(EXIT_FAILURE);
      }
      //  HANDLE PARENT PROCESS
      else {
        int status;
        close(pipefd[1]);
        close(pipefd2[1]);
        fd_set read_fds;
        int fd_stdout = pipefd[0];
        int fd_stderr = pipefd2[0];

        char RBUF[1024];
        int bytes_read_out = 0, bytes_read_err = 0;

        while (fd_stderr != -1 || fd_stdout != -1){
          FD_ZERO(&read_fds);
          int max_fd = -1;

          if (fd_stdout != -1) {
            FD_SET(fd_stdout, &read_fds);
            if (fd_stdout > max_fd) max_fd = fd_stdout;
          }
          if (fd_stderr != -1) {
            FD_SET(fd_stderr, &read_fds);
            if (fd_stderr > max_fd) max_fd = fd_stderr;
          }

          int activity = select(max_fd+1, &read_fds, NULL, NULL, NULL);
          if (activity < 0) {
            if (errno == EINTR) continue;
            perror("selection mein error");
            break;
          }
          else if (activity == 0) continue;

          if (fd_stdout != -1 && FD_ISSET(fd_stdout, &read_fds)) {
            int bytes_read = read(fd_stdout, funcOutput+bytes_read_out, sizeof(funcOutput)-1-bytes_read_out);
            bytes_read_out += bytes_read;
            if (bytes_read == 0) {
              close(pipefd[0]);
              fd_stdout = -1;
            }
          }

          if (fd_stderr != -1 && FD_ISSET(fd_stderr, &read_fds)) {
            int bytes_read = read(fd_stderr, funcError+bytes_read_err, sizeof(funcError)-1-bytes_read_err);
            bytes_read_err += bytes_read;
            if (bytes_read == 0) {
              close(pipefd2[0]);
              fd_stderr = -1;
            }
          }
        }

        /*ssize_t bytes_read_total = 0;
        ssize_t current_bytes_read;

        
        while (bytes_read_total < sizeof(execBuff) - 1 && 
               (current_bytes_read = read(pipefd[0], execBuff + bytes_read_total, sizeof(execBuff) - 1 - bytes_read_total)) > 0) {
            bytes_read_total += current_bytes_read;
        }

        execBuff[bytes_read_total] = '\0';
        close(pipefd[0]);
        waitpid(pid, &status, 0);
        //printf("%s\n", execBuff);
        strcpy(funcOutput, execBuff);*/
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
  free(pathVar);
  return retVal;
}

int typef(char **inputt, char *buff) {
  for (int i = 1; i < maxBuff, inputt[i] != NULL; i++) {
    char *functionName = inputt[i];
    int found = 0;
    for (int i = 0; i < sizeof(func)/sizeof(func[0]); i++) {
      if (!strcmp(functionName, commands[i])) {sprintf(funcOutput, "%s is a shell builtin\n", commands[i]); found = 1; break;}
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
      sprintf(funcOutput, "%s: not found\n", functionName);
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
  sprintf(funcOutput, "%s\n", buff+5);
  return default_return;
}
/*
  COMPLETER FUNCTION
*/
char *generatoR(const char *input, int state) {
  static char *pathVar = NULL;
  struct stat st;
  if (state == 0) {
    /*if (matchList) {
      for (int i = 0; matchList[i]; i++) free(matchList[i]);
      free(matchList);
      matchList = NULL;
    }*/
    listIndex = 0;
    pathVar = strdup(getenv("PATH"));
    tokken = strtok(pathVar, ":");
    if (currentDirStream) closedir(currentDirStream);
  }
  while (matchList[listIndex]) {
    if (!strncmp(matchList[listIndex], input, strlen(input)))  
      return strdup(matchList[listIndex++]);
    listIndex++;
  }
  static struct dirent *entry;
  
  while (tokken != NULL) {
    if (!currentDirStream) currentDirStream = opendir(tokken);
    if (!currentDirStream) {tokken = strtok(NULL, ":"); continue;}
    entry = readdir(currentDirStream);
    while (entry != NULL) {
      if (!strncmp(entry->d_name, input, strlen(input))) {return strdup(entry->d_name);}
      entry = readdir(currentDirStream);
    }
    tokken = strtok(NULL, ":");
    if(currentDirStream) {closedir(currentDirStream); currentDirStream = NULL;}
  }
  free(pathVar);
  return NULL;
}
char **completerFn(const char *input, int start, int end) {
  rl_attempted_completion_over = 1;
  char **a = rl_completion_matches(input, generatoR);
  return a;
}
