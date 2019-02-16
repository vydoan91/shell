//
//  main.c
//  413 hw1
//
//  Created by VY DOAN on 2/6/19.
//  Copyright © 2019 VY DOAN. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

typedef struct pid pid;
struct pid{
    pid_t pid;
    char *commandName;
    char *stt;
};

// MAIN PROGRAM
int main(int argc, const char * argv[]) {
    char line[1024];
    char *args[64];
    int status;
    pid_t childpid;
    pid_t wtpid;
    pid bgList[10];
    int counter = 0;
    
    while (1) {
        signal(SIGINT, SIG_IGN);

        /* waitpid() returns a PID on success */
        while((wtpid = waitpid(-1, &status, WNOHANG)) > 0) {
            printf("BG [%d] exited with status %d\n", wtpid, WEXITSTATUS(status));
            /* here you can remove the pid from your jobs list */
            for(int i = 0; i < counter; i++) {
                if(wtpid == bgList[i].pid) {
                    bgList[i].stt = "Exited";
                }
            }
        }
        int isBg = 0; // 0 means forground, 1 means background
        
        // ask for user input
        printf("tish>>");
        gets(line);
        if(strlen(line) == 0) {
            continue;
        }
        
        // parse in user's input
        int index_args = 0;
        args[index_args] = strtok(line, " ");
        while (args[index_args] != NULL) {
            index_args++;
            args[index_args] = strtok(NULL, " ");
        }
        index_args--;
        
        if(strcmp(args[index_args],"&") == 0) {
            isBg = 1;
            args[index_args] = 0;
        }
        
        // INTERNAL COMMAND
        // bye command
        if (strcmp(args[0], "bye") == 0) {
            for(int j = 0; j < counter; j++) {
                if(strcmp(bgList[j].stt,"Exited") == 0) {
                    continue;
                }
                int killResult = kill(bgList[j].pid, SIGTERM);
                if(killResult == 0){
                    printf("BG <%jd> killed.\n", (intmax_t) bgList[j].pid);
                } else {
                    printf("cannot kill.\n");
                }
            }
            exit(0);
        }
        
        // jobs command
        if (strcmp(line, "jobs") == 0) {
            for(int j = 0; j < counter; j++) {
                printf("<%jd>", (intmax_t) bgList[j].pid);
                printf("<%s>", bgList[j].commandName);
                printf("<%s>\n", bgList[j].stt);
            }
            continue;
        }
        
        // kill command
        if (strcmp(args[0], "kill") == 0) {
            int killResult = kill(atoi(args[1]), SIGTERM);
            if(killResult == 0){
                printf("BG <%s> is killed.\n", args[1]);
                for(int i = 0; i < counter; i++) {
                    if(atoi(args[1]) == bgList[i].pid) {
                        bgList[i].stt = "Exited";
                    }
                }
            } else {
                printf("Kill failed.");
            }
            continue;
        }
        
        // EXECUTE EXTERNAL COMMAND
        childpid = fork();
        if(childpid < 0){
            printf("error: fork()\n");
        } else if (childpid == 0) { //child process
            signal(SIGINT, SIG_DFL);
            if(isBg == 1) {
                setpgid(0, 0);
            }
            execvp(args[0], args);
            if (execvp(args[0], args) == -1) {
                perror("Exec failed");
                continue;
            }
            exit(0);
        } else { //parent process
            if(isBg == 0) { //not a background process
                wtpid = wait(&status);
                printf("[%d] exited with status %d\n", wtpid, (status >> 8));
            } else { // a bacground process
                // add pid to map
                bgList[counter].pid = childpid;
                bgList[counter].commandName = strdup(args[0]);
                bgList[counter].stt = "In background";
                counter++;
                printf("BG [%d] started\n", childpid);
            }
        }
    }
    return 0;
}

