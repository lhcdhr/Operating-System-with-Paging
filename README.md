# Operating System with Paging
 
## Description
A simple operating system implemented with C. 
- To compile, use command `make mysh framesize=X varmemsize=Y`, with `X` corresponding to the size of the frame store, and `Y` representing the size of the variable store in the OS Shell memory. 
- To run the OS in interactive mode, use `./mysh`. 
- The OS also supports batch mode that takes a file as input, use `./mysh < input.txt`.
- Supports basic command, including `help` `quit` `print` `set` `echo` `run` `exec` `my_ls` `resetmem`, and the commands are case sensitive.

## Paging
- The memory management for this simple OS is based on demand paging, following LRU replacement policy.
- A directory will be created as backing store for paging, and it will be deleted when the OS Shell received `quit` command.
- The shell memory is splitted into two parts, one for frames, and one for variables
- Each page consists 3 lines of code.
- When loading scripts to frame, the scripts are copied and sliced into pages and stored in the backing store directory. The frame store will keep track of the program and its pages. The new page is placed at the first available spot of the frame store.
- PCB will contain a page table, keeping track of the loaded pages and their location in the frame store. 
- Initlally, each program will have at least 2 pages loaded into the memory. Requesting page not yet in memory will result in a page fault. 
- Upon the triggerring of a page fault, the currently running program will be placed at the end of the ready queue, and the scheduler will select the next program to run. The requested page will be loaded from the backing store to the frame. If there is a free spot, then it will be loaded there. If not, then the least recently used page will be replaced with the requested page, and a message will be displaced as follows:
`Page fault! Victim page contents:
<the contents of the page, line by line> 
End of victim page contents.` 
- `run` and `exec` both follows the paging policy. This is a different implementation compared to the version focusing on scheduling policies.

## Command
### `help`
- `help`
- Display all the commands and how to use them.
### `quit`
- `quit`
- Terminate the OS Shell, and delete the backing store directory and all its contents.
### `set`
- `set VAR STRING`
- If `VAR` already exists, then overwrite the previously assigned one with `STRING`.
- If not, assign `STRING` to variable `VAR`.
### `print`
- `print VAR`
- Display the value assigned to `VAR`.
- If `VAR` does not exist in memory, then error message will be displayed as `Variable does not exist`
### `echo`
- `echo STRING` or `echo $VAR`
- If the token is a `STRING`, then it will be displayed in the next line.
- If the token is preceded by a `$`, then the assigned value of `VAR` will be displayed. If no such `VAR` exists in the OS Shell memory, then it will display an empty line.
### `run`
- `run SCRIPT`
- Assuming the file `SCRPIT` exists in the current directory, this command sends each line of the file to the intepreter.
- Each line of the command is treated as a command.
- If error occurs when running, the error will be displayed and the script continue running.
- The code of `SCRIPT` is loaded in to the OS Shell memory line by line.
- A PCB keeps track of the PID, the location in memory where the script is stored, and the current inscruction to execute.
- The program is added to the ready queue.
- When `SCRIPT` terminates, the stored code is cleaned from the OS Shell memory.
### `exec`
- `exec P1 P2 P3 POLICY`
- This command can execute up to 3 programs concurrently following the given scheduling policy `POLICY`.
- The scheduling policies only include Round Robin `RR` for this version to simplify the process. 
- The code of each process is loaded following the paging policy, and PCBs are created just like in `run` command.
- PCBs are added to the ready queue, and the order and updating of the queue is determined by the scheduling policies.
- The program that finished executing will be removed from the queue, and the stored code in the memory will be cleaned.
### `my_ls`
- `my_ls`
- List all files in the current directory in alphabetical order.
