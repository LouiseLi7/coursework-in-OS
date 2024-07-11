#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>

#define INT_MAX 2147483647
#define CMDLINE_MAX 512

/*Defining a char array data structure which can help get arguemnts through fgets().*/
struct command {
    char *shell_argv[CMDLINE_MAX];
    int number_argv;
};

typedef struct command Struct;


/*
The return value type 'command' contains the combination of command and its arguments. 
Each argument is a single value in the array.
*/
Struct returnargs (char cmd[]){
    Struct argv;
    int number_argv = 0;
    char *token = strtok(cmd, " ");
    while(token!=NULL){
        argv.shell_argv[number_argv] = token;
        token = strtok(NULL, " ");
        number_argv++;//number of arguments increases and put them into shell_argv[]
    }
    argv.number_argv = number_argv;
    argv.shell_argv[number_argv++] = NULL;
    return(argv);
}

int Redirection(Struct s, char redirect[], char *filename, int std_err){
    /*Redirection process using fork()*/
    int fd;
    pid_t pid_red= fork();
    if(pid_red == 0){
        fd = open(filename, O_WRONLY|O_CREAT, 0644);
        if (fd < 0){
            return INT_MAX;
        }
        if(std_err==1){
            dup2(fd, STDERR_FILENO);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
        /*Error: command not found*/
        int status_code = execvp(s.shell_argv[0],s.shell_argv);
        if (status_code == -1) {
            fprintf(stderr, "Error: command not found\n");
            exit(EXIT_FAILURE);
        }
    }//child process
    else if(pid_red > 0){
        int status;
        waitpid(pid_red, &status, 0);
        return(WEXITSTATUS(status));
    }//parent process
    else{
        perror("fork");
        exit(1);
    }
    return 0;
}

/*Pipeline function built for one pipe sign.*/
void PipeLine1(char cmd[], Struct s1, Struct s2, int std_err_1){
    int filedes[2];
    int status_code = 0;
    pipe(filedes);/*to create pipe*/

    pid_t pid_1 = fork();
    if (pid_1 == 0) {
        if(std_err_1==1){
            dup2(filedes[1], STDERR_FILENO);
        }
        dup2(filedes[1], STDOUT_FILENO);/*connect filedes and stdout through pipe, write */
        close(filedes[1]);
        close(filedes[0]);/*close unnecessary file*/
        /*Error: command not found*/
        status_code = execvp(s1.shell_argv[0], s1.shell_argv);
        if (status_code == -1) {
            status_code = 0;
            fprintf(stderr, "Error: command not found\n");
            exit(EXIT_FAILURE);
        }
    } else {

        pid_t pid_2 = fork();//fork() in parent process to execute another command

        if (pid_2 == 0) {

            dup2(filedes[0], STDIN_FILENO);/*connect filedes and stdin through pipe, read*/
            close(filedes[0]);
            close(filedes[1]);
            /*Error: command not found*/
            status_code = execvp(s2.shell_argv[0], s2.shell_argv);
            if (status_code == -1) {
                status_code = 0;
                fprintf(stderr, "Error: command not found\n");
                exit(EXIT_FAILURE);
            }
        } else {
            close(filedes[0]);
            close(filedes[1]);/*close files in parent process*/
            int status_1;
            int status_2;
            waitpid(pid_1, &status_1, 0);
            waitpid(pid_2, &status_2, 0);/*wait child process*/
            fprintf(stderr, "+ completed '%s' [%d][%d]\n", cmd,
                                                           WEXITSTATUS(status_1),
                                                           WEXITSTATUS(status_2));
        }
    }
}

/*Pipeline function built for two pipe signs.*/
void PipeLine2(char cmd[], Struct s1, Struct s2, Struct s3, int std_err_1, int std_err_2){
    int filedes[2];
    int status_code;
    pipe(filedes);
    //fork in the to connect output and file_write
    pid_t pid_1 = fork();
    if (pid_1 == 0) {
        if(std_err_1==1){
            dup2(filedes[1], STDERR_FILENO);
        }
        dup2(filedes[1], STDOUT_FILENO);
        close(filedes[1]);
        close(filedes[0]);
        /*Error: command not found*/
        status_code = execvp(s1.shell_argv[0], s1.shell_argv);
        if (status_code == -1) {
            status_code = 0;
            fprintf(stderr, "Error: command not found\n");
            exit(EXIT_FAILURE);
        }
    } else {
        int filedes_2[2];
        pipe(filedes_2);

        pid_t pid_2 = fork();

        if (pid_2 == 0) {

            dup2(filedes[0], STDIN_FILENO);
            if(std_err_2==1){
                dup2(filedes_2[1], STDERR_FILENO);
            }
            dup2(filedes_2[1], STDOUT_FILENO);/*connect filedes_2 and stdout through pipe, write */
            close(filedes[0]);
            close(filedes[1]);
            close(filedes_2[1]);
            close(filedes_2[0]);
            /*Error: command not found*/
            status_code = execvp(s2.shell_argv[0],s2.shell_argv);
            if (status_code == -1) {
                status_code = 0;
                fprintf(stderr, "Error: command not found\n");
                exit(EXIT_FAILURE);
            }
        } else {

            close(filedes[0]);
            close(filedes[1]);
            /*similar process as PipeLine1 and PipeLine2, to connect file and input*/
            pid_t pid_3 = fork();
            if(pid_3 ==0){
                dup2(filedes_2[0], STDIN_FILENO);/*connect filedes_2 and stdin through pipe, read */
                close(filedes_2[0]);
                close(filedes_2[1]);
                /*Error: command not found*/
                status_code = execvp(s3.shell_argv[0], s3.shell_argv);
                if (status_code == -1) {
                    status_code = 0;
                    fprintf(stderr, "Error: command not found\n");
                    exit(EXIT_FAILURE);
                }
            }
            else{
                close(filedes_2[0]);
                close(filedes_2[1]);
                int status_1;
                int status_2;
                int status_3;
                waitpid(pid_1, &status_1, 0);
                waitpid(pid_2, &status_2, 0);
                waitpid(pid_3, &status_3, 0);
                fprintf(stderr, "+ completed '%s' [%d][%d][%d]\n", cmd,
                                                                   WEXITSTATUS(status_1),
                                                                   WEXITSTATUS(status_2),
                                                                   WEXITSTATUS(status_3));
            }
        }
    }
}


/*Pipeline function built for three pipe signs.*/
void PipeLine3(char cmd[], Struct s1, Struct s2, Struct s3, Struct s4, int std_err_1, int std_err_2,int std_err_3){
    int filedes[2];
    int status_code;
    pipe(filedes);

    pid_t pid_1 = fork();
    if (pid_1 == 0) {
        if(std_err_1==1){
            dup2(filedes[1], STDERR_FILENO);
        }
        dup2(filedes[1], STDOUT_FILENO);
        close(filedes[1]);
        close(filedes[0]);
        /*Error: command not found*/
        status_code = execvp(s1.shell_argv[0], s1.shell_argv);
        if (status_code == -1) {
            status_code = 0;
            fprintf(stderr, "Error: command not found\n");
            exit(EXIT_FAILURE);
        }
    } else {
        int filedes_2[2];
        pipe(filedes_2);

        pid_t pid_2 = fork();

        if (pid_2 == 0) {
            if(std_err_2==1){
                dup2(filedes_2[1], STDERR_FILENO);
            }
            dup2(filedes_2[1], STDOUT_FILENO);
            close(filedes[0]);
            close(filedes[1]);
            close(filedes_2[1]);
            close(filedes_2[0]);
            /*Error: command not found*/
            status_code = execvp(s2.shell_argv[0], s2.shell_argv);
            if (status_code == -1) {
                status_code = 0;
                fprintf(stderr, "Error: command not found\n");
                exit(EXIT_FAILURE);
            }
        } else {/*similar to pipeline2*/
            int filedes_3[2];
            pipe(filedes_3);
            close(filedes[0]);
            close(filedes[1]);
            pid_t pid_3 = fork();
            if(pid_3 == 0){
                dup2(filedes_2[0], STDIN_FILENO); /*connect filedes_2 and stdin through pipe, read */
                if(std_err_3==1){
                    dup2(filedes_3[1], STDERR_FILENO);
                }
                dup2(filedes_3[1], STDOUT_FILENO);/*connect filedes_3 and stdout through pipe, write */
                close(filedes_2[0]);
                close(filedes_2[1]);
                close(filedes_3[0]);
                close(filedes_3[1]);
                /*Error: command not found*/
                status_code = execvp(s3.shell_argv[0],s3.shell_argv);
                if (status_code == -1) {
                    status_code = 0;
                    fprintf(stderr, "Error: command not found\n");
                    exit(EXIT_FAILURE);
                }
            } 
            else{
                close(filedes_2[0]);
                close(filedes_2[1]);
                pid_t pid_4 = fork();
                if(pid_4 == 0){
                    dup2(filedes_3[0], STDIN_FILENO); /*connect filedes_3 and stdin through pipe, read*/
                    close(filedes_3[0]);
                    close(filedes_3[1]);
                    /*Error: command not found*/
                    status_code = execvp(s4.shell_argv[0], s4.shell_argv);
                    if (status_code == -1) {
                        status_code = 0;
                        fprintf(stderr, "Error: command not found\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else{
                close(filedes_3[0]);
                close(filedes_3[1]);/*close files in parent process*/
                int status_1;
                int status_2;
                int status_3;
                int status_4;
                waitpid(pid_1, &status_1, 0);
                waitpid(pid_2, &status_2, 0);
                waitpid(pid_3, &status_3, 0);
                waitpid(pid_4, &status_4, 0);/*wait for child process */
                fprintf(stderr, "+ completed '%s' [%d][%d][%d][%d]\n",cmd,
                WEXITSTATUS(status_1),WEXITSTATUS(status_2),WEXITSTATUS(status_3),
                WEXITSTATUS(status_4));
                }
            }
        }
    }
}

/*Besides specific commands such as pwd and cd which can realize through functions,other commands need exec to realize such as ls.*/
void RegularCommands(Struct s,char cmd[]){
    pid_t pid;
    pid = fork();
    if(pid == 0){
        /*Error: command not found*/
        int status_code = execvp(s.shell_argv[0],s.shell_argv);
        if (status_code == -1) {
            fprintf(stderr,"Error: command not found\n");
            exit(EXIT_FAILURE);
        }
    }
    else if(pid>0){
        int status;
        waitpid(pid,&status,0);
        fprintf(stderr, "+ completed '%s' [%d]\n",cmd,WEXITSTATUS(status));//parent
    }
    else{
        perror("fork");
        exit(1);
    }
}

/*Main function that we will run*/
int main(void)
{
        char cmd[CMDLINE_MAX];
        char backupcmd[CMDLINE_MAX];
        //avoid some conditions that string cannot be used caused by strtok()

        while (1) {
                char *nl;
                int retval;

                /* Print prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);


                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }


                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /*Command struct. To create a backupcmd in order to keep track of the original input.*/
                memcpy(backupcmd,cmd,sizeof(cmd));
                Struct argv;

                /* Redirection */
                if(strstr(cmd,">")){
                    /*Divide command into input and output*/
                    int std_err = 0;
                    char *cmd_1 = strstr(cmd,">");
                    int pos = cmd_1-cmd;
                    char redirect[CMDLINE_MAX];
                    memcpy(redirect,cmd,pos);
                    /*To get the output filename*/
                    char *filename = &cmd[pos+1];
                    if(strstr(cmd,">&")){
                        filename = &cmd[pos+2];
                        std_err = 1;
                    }
                    /* Error : mislocated output redirection*/
                    if(strstr(filename, "|") != NULL){
                        fprintf(stderr, "Error: mislocated output redirection\n");
                        continue;
                    }

                    filename = strtok(filename," ");

                    /*Remove spaces*/
                    redirect[pos] = '\0';
                    argv = returnargs(redirect);

                    /* Error : too many args > 16*/
                    if (argv.number_argv > 16){
                        fprintf(stderr, "Error: too many process arguments\n");
                        continue;
                    }

                    /*Error: no input*/
                    if(strtok(redirect, " ")==NULL){
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                    }
                    /*Error: no output*/
                    if(strtok(filename, " ")==NULL){
                        fprintf(stderr, "Error: no output file\n");
                        continue;
                    }

                    /*Redirection process using fork()*/
                    int return_value = Redirection(argv,redirect,filename,std_err);

                    /* Error : cannot open output file*/
                    if (return_value == INT_MAX){
                        fprintf(stderr, "Error: cannot open output file\n+ completed '%s' [%d]\n",backupcmd,return_value);
                        continue;
                    }
                    fprintf(stderr, "+ completed '%s' [%d]\n",backupcmd,return_value);

                }

                /* Pipe command*/
                else if(strstr(cmd,"|")){
                    int std_err_1 = 0;
                    char *cmd_1 = strstr(cmd,"|");
                    int pos = cmd_1-cmd;
                    /*Dividing into different parts of pipes*/
                    char cmd_p1[CMDLINE_MAX]; /*part 1 command*/
                    char *cmd_p2=&cmd[pos+1]; /*part 2 command*/
                    memcpy(cmd_p1,cmd,pos);
                    //strcpy(cmd_p2,&cmd[pos+1]);
                    cmd_p1[pos] = '\0';
                    Struct argv_cmd_p1; /*part 1 command parsing*/
                    Struct argv_cmd_p2; /*part 2 command parsing*/
                    argv_cmd_p1 = returnargs(cmd_p1);
                    /*Error: missing command*/
                    if(strtok(cmd_p1, " ")==NULL){
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                    }
                    /* Error : too many args > 16*/
                    if (argv_cmd_p1.number_argv > 16){
                        fprintf(stderr, "Error: too many process arguments\n");
                        continue;
                    }
                    if(strstr(cmd,"|&")){
                        cmd_p2 = &cmd[pos+2];
                        std_err_1 = 1;
                    }
                    if(!strstr(cmd_p2,"|")){
                        /*Pipe 1*/
                        /*Error: missing command*/
                        argv_cmd_p2 = returnargs(cmd_p2);
                        if(strtok(cmd_p2, " ")==NULL){
                            fprintf(stderr, "Error: missing command\n");
                            continue;
                        }
                        /* Error : too many args > 16*/
                        if (argv_cmd_p2.number_argv > 16){
                            fprintf(stderr, "Error: too many process arguments\n");
                            continue;
                        }
                        PipeLine1(backupcmd, argv_cmd_p1,argv_cmd_p2,std_err_1);
                    }
                    else{
                        Struct argv_cmd_p3; /*part 3 command parsing*/
                        int std_err_2 = 0;
                        char *cmd_2 = strstr(cmd_p2,"|");
                        pos = cmd_2-cmd_p2;
                        /*Dividing into different parts of pipes*/
                        char cmd_p3[CMDLINE_MAX]; /*part 2 command*/
                        char *cmd_p4 = &cmd_p2[pos+1]; /*part 3 command*/
                        memcpy(cmd_p3,cmd_p2,pos);
                        cmd_p3[pos] = '\0';
                        argv_cmd_p2 = returnargs(cmd_p3);
                        /* Error : missing command*/
                        if(strtok(cmd_p3, " ")==NULL){
                            fprintf(stderr, "Error: missing command\n");
                            continue;
                        }
                        /* Error : too many args > 16*/
                        if (argv_cmd_p2.number_argv > 16){
                            fprintf(stderr, "Error: too many process arguments\n");
                            continue;
                        }
                        if(strstr(cmd_p2,"|&")){
                            cmd_p4 = &cmd_p2[pos+2];
                            std_err_2 = 1;
                        }/*no use of cmd_p2 any more*/
                        /*Pipe 2*/
                        if(!strstr(cmd_p4,"|")){
                            argv_cmd_p3 = returnargs(cmd_p4);
                            /* Error : missing command*/
                            if(strtok(cmd_p4, " ")==NULL){
                                fprintf(stderr, "Error: missing command\n");
                                continue;
                            }
                            /* Error : too many args > 16*/
                            if (argv_cmd_p3.number_argv > 16){
                                fprintf(stderr, "Error: too many process arguments\n");
                                continue;
                            }
                            PipeLine2(backupcmd,argv_cmd_p1,argv_cmd_p2,argv_cmd_p3,std_err_1,std_err_2);
                        }
                        else{
                            /*Pipe 3*/
                            char *cmd_3 = strstr(cmd_p4,"|");
                            pos = cmd_3-cmd_p4;
                            /*Dividing into different parts of pipes*/
                            char cmd_p5[CMDLINE_MAX]; /*part 3 command*/
                            char *cmd_p6 =&cmd_p4[pos+1]; /*part 4 command*/
                            memcpy(cmd_p5,cmd_p4,pos);
                            cmd_p5[pos] = '\0';
                            Struct argv_cmd_p4; /*part 4 command parsing*/
                            int std_err_3 = 0;
                            if(strstr(cmd_p4,"|&")){
                                cmd_p6 = &cmd_p4[pos+2];
                                std_err_3 = 1;
                            }/*cmd_p4 is not in used any more*/
                            argv_cmd_p3 = returnargs(cmd_p5);
                            /* Error : missing command*/
                            if(strtok(cmd_p5, " ")==NULL){
                                fprintf(stderr, "Error: missing command\n");
                                continue;
                            }
                            /* Error : too many args > 16*/
                            if (argv_cmd_p3.number_argv > 16){
                                fprintf(stderr, "Error: too many process arguments\n");
                                continue;
                            }
                            argv_cmd_p4 = returnargs(cmd_p6);
                            /* Error : missing command*/
                            if(strtok(cmd_p6, " ")==NULL){
                                fprintf(stderr, "Error: missing command\n");
                                continue;
                            }
                            /* Error : too many args > 16*/
                            if (argv_cmd_p4.number_argv > 16){
                                fprintf(stderr, "Error: too many process arguments\n");
                                continue;
                            }
                            PipeLine3(backupcmd,argv_cmd_p1,argv_cmd_p2,argv_cmd_p3,argv_cmd_p4,std_err_1,std_err_2,std_err_3);
                        }
                    }
                }


                /* Builtin command */
                else{
                    argv = returnargs(cmd);
                    char *token = argv.shell_argv[0];/*To get the first argument of input: function to run*/
                    if (!strcmp(token, "exit")) {
                            /*Exit command*/
                            fprintf(stderr, "Bye...\n");
                            fprintf(stderr,"+ completed '%s' [0]\n",cmd);
                            break;
                    }
                    else if (!strcmp(token, "pwd")) {
                            /*pwd command*/
                            getcwd(cmd, sizeof(cmd));
                            printf("%s\n", cmd);
                            fprintf(stderr,"+ completed '%s' [0]\n",backupcmd);
                    }
                    else if (!strcmp(token,"cd")){
                            /*cd command*/
                            token = argv.shell_argv[1];
                            /* Error : cannot cd into directory > 16*/
                            if (chdir(token) != 0){
                                fprintf(stderr, "Error: cannot cd into directory\n");
                                fprintf(stderr,"+ completed '%s' [0]\n",backupcmd);
                                continue;
                            }
                            fprintf(stderr,"+ completed '%s' [0]\n",backupcmd);
                    }
                    else if (!strcmp(token,"sls")){
                            /*sls command*/
                            DIR *directory;
                            struct dirent *dp;
                            struct stat data;/*library structure*/
                            directory = opendir(".");//get directory
                            while((dp =readdir(directory))!=NULL){
                                /*print information of files in the directory and hide the hidden files*/
                                if(dp->d_name[0]!='.'){
                                    stat(dp->d_name,&data);
                                    printf("%s (%lld bytes)\n",dp->d_name,(long long)data.st_size);
                                }
                            }
                            closedir(directory);
                            fprintf(stderr,"+ completed '%s' [0]\n",backupcmd);

                    }
                    
                    else {
                        /* Regular command left*/
                        /* Error : too many args > 16*/
                        if (argv.number_argv > 16){
                            fprintf(stderr, "Error: too many process arguments\n");
                            continue;
                        }
                        RegularCommands(argv,backupcmd);
                    }
                }
        }

        return EXIT_SUCCESS;
}
