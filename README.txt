-----------------
**** README ****
-----------------
 
Files included:
- this README.txt
- Makefile
- source file my_shell.c
 
—————————————
Instructions
—————————————
 
In order to execute this program, type "make" in the command line.
Then type “./my_shell”.

———————————
Description
———————————
 
This program allows the user to run a simple shell on their UNIX machines.

——————————————
Implementation
—————————————— 

I implemented my shell in C language. When thinking of the structure of my program, I decided to split the main functionality of the shell into several functions. I have a separate function for reading user’s input (shell_read), splitting it into arguments (shell_split) and executing it (shell_execute), which calls another function that launches the processes (shell_launch). Also, I created separate functions for basic built-in commands, such as “cd” (shell_cd), “exit” (shell_exit), “pwd” (shell_pwd), “history” (shel_history) and “hc” (shell_hc). My main function contains declaration of some variables and the shell loop, which executes the reading, splitting and executing user’s commands, as well as recording them in the history. The loop is maintained by a flag variable “status”, which indicates whether the user wants to exit the shell.

——————————
Resources
——————————

I visited these websites in order to see some examples of C functions and Linux manual pages:
	- https://linux.die.net/man/
	- https://www.tutorialspoint.com/cprogramming
	- http://tldp.org/LDP/lpg/node1.html
	- https://www.gnu.org/software/libc/manual/html_node/index.html#SEC_Contents

Created by Alina Lapushkina on 2017-02-10.
Copyright (c) 2017 Alina Lapushkina. All rights reserved.
