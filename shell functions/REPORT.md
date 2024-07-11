# sshell.c: simple shell implementation

## Summary
In this project, we aim to build a simple shell that can realize some commands such as build-in methods such as exit, pwd, cd, ls, echo, and so on. It also supports the redirection, pipeline, and new features including standard error output and sls, which can show up the storage in bytes. 

## Implementation
The implementation of this program follows this logic：

1. Read the standard input command through `fgets()` and parsing it into meaningful arguments
2. Find out all of the required commands we need to realize and the condition to trigger them
3. In each command, use functions inside library or building new functions to 
get the ideal output

### Reading standard input

The sketch has already given the code of `fgets()` so that we can read the standard input as a char array. However, we need to divide it into different arguments to get common `argv[]`, so a parsing function is needed. We create a new type `command`(`Struct`), which can help the command array transfrom into the usual type of `argv[]`. The new function is `returnargs`, which can return `argv[]` after running. In order to achieve the goal, we have used the `strtok` function, which can remove the extra spaces between words. Through the while loop, each argument is put into the pointer char array. 

### Finding out all of the required commmands and triggering them

In this project, there are lots of commands we need to realize. According to the file, the mentioned commands include exit, cd, pwd, output redirection, piping, standard error redirection, and sls. There are also some commands that come from external executable programs. We use the `if...else if...else` structure that can identify what is the first argument, which is the command function.The condition to trigger each if requirement is the command, so we use `strcmp` to compare the command we receive and the condition string. If one has satisfies one condition, it will not run any of the other codes of this loop. After running and displaying the completed message, it will go to another loop and start a new line to receive inputs.
Fortunately, most of the commands can be successfully implemented after using the functions inside the library. 

### Using library functions and building new functions

The first commands we have built are the regular commands that come from external executable programs and the way to implement is to use `fork()` and `exec()`. `fork()` has provided a child process and a parent process, in the child process, we will run the `execvp()` function that will return either standard output or standard error message. The reason why we use it is it does not require the pathname and it finds the executable programs automatically. If there is an error in executing `execvp()`, it will run the `perror()` function and exit. If it runs successfully, it will return status using `WEXITSTATUS(status)` in parent process. `waitpid()` is the function that can make sure parent process will exit later than the child process. 

Secondly, we implement the `exit`,`cd`, `pwd`, and `sls`. These functions can execute successfully just by using library functions. For the last three commands, we should care about directory, so some functions related to the directory path should be used in these conditions. In `exit` process, we just simply `break` it; in `cd` process, we use `chdir()` to go to the new directory; in `pwd` process, we use `gewtcwd()` to get the current directory path. As for `sls`, it is a little bit complicated as it needs to first get the current directory by using `opendir()` and then use `stat()` to get the statsitics of each file inside the directory. Besides, in this condition, we need to hide the hidden files start by `.` and we import stat and dirent from `<sys/stat.h>` and `<dirent.h>`. After printing out the file name and the file size, we should close the directory by using `closedir()`.

The next step is redirection, which is similar to the regular commands running. In the beginning, we find out the input arguments and the output file. char array `redirect` is the input commands and `filename` is the output file. We separate them by the position of `>`. If it is the case for standard error case, which is `>&`, we use the integer variable `std_err` to identify it. After parsing the received the input, we run the new building function `Redirection()` which will return an integer, which is the status output. Besides the regular use of `fork()`,`waitpid()`,`execvp()`, we also use the `dup2()` function which connect the standard output or standard error message to the location of the file. Therefore, the result will write to the file.

The final stage is the use of pipeline. This is relatively hard to implement. We have considered whether to use a while loop or three different functions for the number of pipe signs. We finally decide to use the later one as it will cope less with the `char[]` and the pointers, though it may be long. Similar to the redirection, we use `|` or `|&` to find the pipe signs and divide them into different parts. If the later one still contains other pipe signs, we will continue divide the input into different parts. Finally, the program will find out how many pipe sign so which programs should run. If there is one pipe sign, it will run `PipeLine1()`, if there is two pipe signs. it will run `PipeLine2()` and three for `PipeLine3()`. In each function, we still use `fork()`,`waitpid()`,`execvp()`, and `dup2()`. In the child process, it connects part of file to standard output or standard error and execute. In the parent process, there is another `fork()` and in the parent's new child process, it connects part of the file to standard input so it can read and execute the next command. If there is another pipe sign, it also repeats the writing output process in the beginning: using `dup2()` again and . 

##Testing
In order to test it, when testers finished implementing one function, testers will use the example in html to test. After running all of the examples in the html, testers submitted the file to gradescope to verify. In this process, testers check the format and make it printed in correct order.

## Source
https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm (for the use of strtok)

## License

Copyright 2022, Louise Li, Zhixi Liu
