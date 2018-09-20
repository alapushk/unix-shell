#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> //for chdir(), fork(), exec() and pid_t
#include <sys/wait.h> //for waitpid()
#include <signal.h> //for kill()


//This function reads a line from the standard input:
char *shell_read(){
    char *line = NULL; //used to store the line from the input
    ssize_t buffer = 0;
    getline(&line, &buffer, stdin);//read the line from stdin
    if (line[strlen(line)-1]=='\n') { //remove the new line character from the end
        line[strlen(line)-1] = '\0';
    }
    return line;
}

//This function splits the line from the input into the arguments for further processing:
char **shell_split(char *line){
    int buffer= 64;
    int pos = 0; //position in the array of arguments
    char **arguments = malloc(buffer * sizeof(char*)); //dynamically allocate memory
    char *argument;
    
    argument = strtok(line, " \t\r\n\a"); //tokenize the line using "\t\r\n\a" deliminator
    while (argument != NULL) {
        arguments[pos] = argument; //save each argument's address into the array
        pos++; //increment the position
        
        if (pos >= buffer) { //check whether you ran out of memory space
            buffer = buffer+64; //increment the buffer size
            arguments = realloc(arguments, buffer * sizeof(char *)); //reallocate the array
        }
        argument = strtok(NULL, " \t\r\n\a");
    }
    
    arguments[pos] = NULL; //terminate the array
    return arguments;
}
////// Implementation of the builtin shell commands "exit", "cd", "pwd", "history" and "hc"/////
int shell_exit(char **arguments);
int shell_cd(char **arguments);
int shell_pwd(char **arguments);
int shell_history(char **arguments);
int shell_hc();

char *builtins[] = {"exit","cd", "pwd", "history", "hc"}; //list of the builtin commands

int (*builtins_f[])(char**) = {&shell_exit, &shell_cd, &shell_pwd, &shell_history, &shell_hc}; //array of function pointers to the builtin functions

char *history_buffer[9]; //used to store the last 10 user commands

int shell_exit(char **arguments) {return 0;}//performs exiting the shell

int shell_cd(char **arguments){//performs cd to another folder
    if (arguments[1] == NULL) {
        printf("Expected an argument for cd \n");
    }
    else{
        if(chdir(arguments[1]) != 0){printf("Error executing chdir, please retry \n");}
    }
    return 1;
}

int shell_pwd(char **arguments){//performs printing the current working directory to the user
    long size;
    char *buffer, *current;
    size = pathconf(".", _PC_PATH_MAX);
    if ((buffer = (char*)malloc((size_t)size)) != NULL) {
        current = getcwd(buffer, (size_t)size);
        printf("%s\n", current);
    }
    return 1;
}

int shell_history(char **arguments){//prints out the last 10 history entries to the user
    int i;
    for (i=0; i<10; i++) {
        if (history_buffer[i]!=NULL) {
            printf("%s\n", history_buffer[i]);
        }
    }
    return 1;
}

int shell_hc(){//performs clearing of the history
    int i;//incrementor variable
    
    for(i=0; i<10; i++){
        free(history_buffer[i]);//free each entry
        history_buffer[i] = NULL;//and set it to NULL
    }
    printf("The history was successfully cleared\n");
    return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//This function launches a new process from the user's command:
int shell_launch(char **arguments){
    pid_t my_pid, parent_pid;
    int status;
    
    my_pid = fork(); //create a new process
    if(my_pid < 0){
        printf("Error executing fork()");
    }
    else if (my_pid == 0) {
        execvp(arguments[0], arguments); //child process
        exit(EXIT_FAILURE);
    }
    else{
        do {
            parent_pid = waitpid(my_pid, &status, WUNTRACED); //parent process
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        
    }
    return 1;
}

int shell_pipe(char **arguments){
 
    int fd[2], fd2[2];//the pipes: 0 = output, 1 = input
    int number = 0; //number of commands in the arguments
    int flag = 0; //indicates the end of the loop
    //incrementor variables:
    int h = 0; //first loop
    int i = 0; //second loop
    int j = 0; //second loop
    int k = 0;
    
    pid_t my_pid;
    char *my_args[256];//contains the commands for the pipes
    
    
    while (arguments[h] != NULL) {//calculate the number of commands
        if (strcmp(arguments[h], "|") == 0) {//find the position of the "|"
            number++;//and keep the track of the number of commands
        }
        h++; //increment h
    }
    number++;
    
    while (arguments[j] != NULL && flag != 1) { //go through the arguments array
        k = 0; //reset k to 0
        while (strcmp(arguments[j], "|") != 0) {
            my_args[k] = arguments[j]; //store the command for this iteration
            j++; //increment j
            if (arguments[j] == NULL) {//if no more commands left
                k++; //increment k
                flag = 1; //since it is the end, turn on the flag variable
                break;
            }
            k++; //increment k
        }
        my_args[k] = NULL; //indicates the end
        j++; //increment j
        
        //need to check which iteration it currently is in order to determine which pipes to operate and how:
        if (i%2 != 0) {//if it is an odd iteration
            pipe(fd); //use the 1st pipe
        }
        else{//if it is an even iteration
            pipe(fd2); //use the 2nd pipe
        }
        
        my_pid = fork(); //create a new process
        
        if (my_pid == 0) {
            if (i == 0) {//if it is the first command
                dup2(fd2[1], STDOUT_FILENO); //pipe 2 input/write mode on
            }
            else if (i == (number - 1)){ //if it is the last command
                if (number % 2 != 0) { //and odd number of commands
                    dup2(fd[0], STDIN_FILENO); //pipe 1 output/read mode on
                }
                else{ //or even number of commands
                    dup2(fd2[0], STDIN_FILENO);//pipe 2 output/read moe on
                }
            }
            else{ //if it is a command in the middle
                if (i % 2 != 0) {//and odd iteration
                    dup2(fd2[0], STDIN_FILENO); //pipe 2 output/read mode on
                    dup2(fd[1], STDOUT_FILENO); //pipe 1 input/write mode on
                }
                else{ //or even iteration
                    dup2(fd[0], STDIN_FILENO); //pipe 1 output/read mode on
                    dup2(fd2[1], STDOUT_FILENO); //pipe 2 input/write mode on
                }
                
            }
            if (execvp(my_args[0], my_args) == -1) {//execute command and handle error
                kill(getpid(), SIGTERM);
            }
        }
        //closing the descriptors:
        if (i==0) {//if it is the first command
            close(fd2[1]); //close pipe 2 input/write mode
        }
        else if(i == (number - 1)){//if it is the last command
            if (number % 2 != 0) { //and odd number of commands
                close(fd[0]); //close pipe 1 output/read mode
            }
            else{ //or even number of commands
                close(fd2[0]); //close pipe 2 output/read mode
            }
        }
        else{//if it is a command in the middle
            if (i % 2 != 0) { //and odd iteration
                close(fd2[0]); //close pipe 2 output/read mode
                close(fd[1]); //close pipe 1 input/write mode
            }
            else{//or even iteration
                close(fd[0]); //close pipe 1 output/read mode
                close(fd2[1]); //close pipe 2 input/write mode
            }
        }
        waitpid(my_pid, NULL, 0);
        i++;
    }
    return 1;
}

//This function executes the entered command by checking whether it is one of the builtin commands or, otherwise, launching a new process:
int shell_execute(char **arguments){
    int i; //incrementor variable
    int number = sizeof(builtins)/sizeof(char *); //number of builtins
    int size = sizeof(arguments)/sizeof(char **); //number of arguments
    if (arguments[0] == NULL){ return 1; } //checks whether the user typed an empty command
    
    for (i=0; i<number; i++) {
        if (strcmp(arguments[0], builtins[i]) == 0) { //check whether the entered command is a builtin command
            return (*builtins_f[i])(arguments);//and execute it
        }
    }
    i = 0; //reset i
    while (arguments[i] != NULL) {
        //printf("%s ", arguments[i]);
        if (!(strcmp(arguments[i], "|"))) {//check whether the user command contains pipes
            return shell_pipe(arguments);//and call the pipeline function
        }
        i++;
    }
    return shell_launch(arguments); //otherwise launch a new process
}

//The main function with the shell loop:
int main(int argc, char **argv){
    printf("Hello! Welcome to my shell! Enter your command:\n");
    char *line; //contains the user's command
    char **arguments; //contains the arguments from the entered command
    int status; //flag variable used to maintain the shell loop
    int i, j = 0; //incrementor variables
    
    //set the history to NULL:
    for (i=0; i<10; i++) {
        history_buffer[i] = NULL;
    }
    
    //run the shell loop:
    do{
        printf("shell > "); //print the prompt "shell >"
        line = shell_read(); //read a line one at a time
        //record the history:
        if (j<10) {
            if (!(strcmp(line, "history")) || (!strcmp(line, "hc"))) {
                //do not record it in the history_buffer or increment j
            }
            else{//record it in the history_buffer and increment j
                history_buffer[j] = strdup(line);
                j++;
            }
        }
        else if (j==10) {
            if ((!strcmp(line, "history")) || (!strcmp(line, "hc"))) {
                //do not record it in the history_buffer or increment j
            }
            else{//record it in the history_buffer and increment j
                j = 0;
                history_buffer[j] = strdup(line);
                j++;
            }
        }
        
        arguments = shell_split(line); //split this line into arguments
        status = shell_execute(arguments); //execute these arguments and return a status code
        
        //free the variables for the next user command:
        free(line);
        free(arguments);
    } while (status);
    
    i = shell_hc(); //clear the history
    printf("Thank you for using my shell!\n");
    return EXIT_SUCCESS;
}
