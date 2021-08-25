#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wordexp.h>
int main(int argc,char** argv) {
        bool debug = false;
        if (argv[1] && !strcmp(argv[1], "-debug"))
                debug = true;
        char user_command[200], saved_command[200]; // the entered command line
        char* array_param[50]; // array of the param of the command
        if (debug){
                int pid = getpid();
                printf("INFO: Father started PID[%d]\n", pid);
        }
        printf("Welcome to my mini shell. Type \"exit\" to terminate\n");
        while (true){
                printf("minishell> ");
                gets(user_command);
                strcpy(saved_command, user_command); // saved for printed comment later
                if (!strcmp(user_command, "exit")){
                       if (debug)
                               printf("INFO: Father will terminate\n");
                       exit(0);
                }
                char * array_splited[2];
                char* splited = strtok(user_command, "|"); // get the commands
                for (int i=0; i<2; i++){
                        array_splited[i]=splited;
                        splited=strtok(NULL, "|");
                }
                bool pipe_found = false;
                int fd[2];
                if (array_splited[1]){
                        pipe_found = true;
                        pipe(fd);
                        if (debug){
                                printf("INFO: Pipe detected. Command 1: \"%s\" and Command 2: \"%s\"\n", array_splited[0], array_splited[1]);
                                printf("INFO: Making pipe\n");
                        }
                }
                else if (debug)
                        printf("INFO: No pipe detected, creating child for command \"%s\"\n", array_splited[0]);
                wordexp_t p; //get the params, also the spiceal ones
                wordexp(array_splited[0], &p, 0);
                int pid2 = -1, pid1 = fork();
                if (pipe_found && pid1!=0) pid2 = fork();
                if (pid1 == 0){
                        pid1 = getpid();
                        if (debug)
                                printf("INFO: Child started PID[%d] command \"%s\"\n", pid1, array_splited[0]);
                        if (pipe_found){
                                dup2(fd[1], 1); // replace stdout with write to pipe
                                close(fd[0]); // close unneccery fd for reading
                                close(fd[1]); // close unneccery fd for writing
                        }
                        execvp(p.we_wordv[0], p.we_wordv); //execution
                        printf("%s: command not found\n", saved_command); //if execution failed
                        exit(0);
                } else if (pid2 == 0) {
                        pid2 = getpid();
                        if (debug)
                                printf("INFO: Child started PID[%d] command \"%s\"\n", pid2, array_splited[1]);
                        wordexp_t s; //get the params, also the spiceal ones
                        wordexp(array_splited[1], &s, 0);
                        dup2(fd[0], 0); // make stdin come from pipe
                        close(fd[0]);
                        close(fd[1]);
                        execvp(s.we_wordv[0], s.we_wordv); //execution
                        printf("%s: command not found\n", saved_command); //if execution failed
                        exit(0);
                } else {
                        wordfree(&p);
                        close(fd[0]);
                        close(fd[1]);
                        wait(NULL); // wait to child to finish
                        if (pipe_found){
                                wait(NULL); // wait to another child to finish
                                if (debug){
                                        printf("INFO: Child with PID[%d] terminated\n", pid1);
                                        printf("INFO: Child with PID[%d] terminated\n", pid2);
                                }
                        } else if (debug)
                                printf("INFO: Child with PID[%d] terminated, continue waiting commands\n", pid1);
                }
        }
return 0;
}
