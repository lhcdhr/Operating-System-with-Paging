// Haochen Liu
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 7;

int help();
int quit();
int badcommand();
int badcommandTooManyTokens();
int badcommandFileDoesNotExist();
int badcommandLoadingSameFile();
int set(char* var, char* value);
int resetmem();
int print(char* var);
int run(char* script);
int replacePage(char *pid, int numPage, int target);
int loadPage(char *pid, int numPage);
int exec(char* command_args[], int args_size);
int my_ls();
int echo();
// ensures unique pid
// after each pcb is constructed, 
// this count will increment by 1.
int pid_count = 0;

// struct of PCB
typedef struct pcb
{
	char pid[4];
	int length;
	int pc;
	int age;
	int totalPage;
	int currentPage;//
	int currentFrame;//0,1,2
	int *pagetable;
	struct pcb *next;
}pcb_t;

pcb_t *head;
pcb_t *tail;

int interpreter(char* command_args[], int args_size){
	int i;
	if ( args_size < 1 || args_size > MAX_ARGS_SIZE){
		if (strcmp(command_args[0], "set")==0 && args_size > MAX_ARGS_SIZE) {
			return badcommandTooManyTokens();
		}
		return badcommand();
	}

	for ( i=0; i<args_size; i++){ //strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommand();
	    return help();
	
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) return badcommand();
		return quit();

	} else if (strcmp(command_args[0], "set")==0) {
		//set
		if (args_size < 3) return badcommand();
		char* value = (char*)calloc(1,150);
		char spaceChar = ' ';

		for(int i = 2; i < args_size; i++){
			strncat(value, command_args[i], 30);
			if(i < args_size-1){
				strncat(value, &spaceChar, 1);
			}
		}
		return set(command_args[1], value);
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		
		if (args_size != 2) return badcommand();
		return run(command_args[1]);
	
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size > 2) return badcommand();
		return my_ls();
	
	}else if (strcmp(command_args[0], "echo")==0) {
		if (args_size > 2) return badcommand();
		return echo(command_args[1]);
	
	} 
	else if(strcmp(command_args[0], "exec")==0){
		// check argument length
		if(args_size <=2 || args_size >5) return badcommand();
		// check validity of scheduling policies
		if(strcmp(command_args[args_size-1], "FCFS") != 0 && 
			strcmp(command_args[args_size-1], "SJF") != 0 &&
			strcmp(command_args[args_size-1], "RR") != 0 &&
			strcmp(command_args[args_size-1], "AGING") != 0)
		{
			return badcommand();
		}
		return exec(command_args, args_size);
	}else if(strcmp(command_args[0], "resetmem")==0){
		if (args_size > 2) return badcommand();
		return resetmem();
	}
	else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit(){
	system("rm -rf backingstore");
	printf("%s\n", "Bye!");
	exit(0);
}

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

int badcommandTooManyTokens(){
	printf("%s\n", "Bad command: Too many tokens");
	return 2;
}

int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}
int badcommandLoadingSameFile(){
	printf("%s\n", "Bad command: same file name");
	return 4;
}

int set(char* var, char* value){
	char *link = "=";
	char buffer[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value);
	var_set_value(var, value);
	//showVariable();
	return 0;

}

int resetmem(){
	var_reset();
	return 0;
}

int print(char* var){
	printf("%s\n", var_get_value(var)); 
	return 0;
}

int run(char* script){
	printf("Frame Store Size = %d; Variable Store Size = %d\n", FRAMESIZE, VARMEMSIZE);
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(script,"rt");  // the program is in a file
	if(p == NULL){
		return badcommandFileDoesNotExist();
	}
	fgets(line,999,p);
	pcb_t *pcb_current = NULL;
	pcb_current = malloc(sizeof(pcb_t));
	// only one script, then it is both head and tail
	head = pcb_current;
	tail = pcb_current;
	char new_pid[4];
	sprintf(new_pid, "%d", pid_count);
	strcpy(pcb_current->pid, new_pid);

	int page_count = 0;
	int line_count = 0;
	int write_page_done = -1;	
	// save into backing store
	while(write_page_done == -1){
		// page name format: pid + "-" + page number 	
		char pagename[30] = {};
		strcat(pagename,"backingstore/");
		strcat(pagename, new_pid);
		strcat(pagename, "-");
		char char_page_count[3];
		sprintf(char_page_count, "%d", page_count);
		strcat(pagename, char_page_count);
		FILE *write_page = fopen(pagename, "w");
		for(int i=0;i<3;i++){
			int length = strlen(line);
			if(line[length-1]!='\n'){
				line[length] = '\n';
				line[length+1] = '\0';
			}
			fprintf(write_page,"%s",line);
			memset(line, 0, sizeof(line));
			line_count++;
			if(feof(p)){
				write_page_done=1;
				break;
			}
			fgets(line,999,p);
		}
		fclose(write_page);
		page_count++;	
	}
	fclose(p);
	pcb_current->length = line_count;
	pcb_current->pc = 0;
	pid_count++;
	pcb_current->totalPage=page_count;
	pcb_current->currentPage=0;
	pcb_current->currentFrame=0;
	pcb_current->pagetable=malloc(pcb_current->totalPage*sizeof(int));
	// initialize as -1 - means page not yet in memory
	for(int i=0;i<pcb_current->totalPage;i++){
		pcb_current->pagetable[i]=-1;
	}
	// save first two pages into shell memory
	if(pcb_current->length<=3){
		pcb_current->pagetable[0]=loadPage(pcb_current->pid, 0);
		updateLRU(pcb_current->pagetable[0]);
	}
	else{
		pcb_current->pagetable[0]=loadPage(pcb_current->pid, 0);
		updateLRU(pcb_current->pagetable[0]/3);
		pcb_current->pagetable[1]=loadPage(pcb_current->pid, 1);
		updateLRU(pcb_current->pagetable[1]/3);
	}
	//showLRU();
	//for(int i=0;i<pcb_current->totalPage;i++){
	//	printf("page %d location starts from %d\n",i,pcb_current->pagetable[i]);
	//}
	//showMemory();  
	// run the script, based on LRU logic.
	//printf("total page number is %d\n",pcb_current->totalPage);
	while(pcb_current->currentPage < pcb_current->totalPage){
		//showLRU();
		int page_index = pcb_current->pagetable[pcb_current->currentPage];
		// page fault
		if(page_index == -1){
			int possible_empty = empty_spot();
			// if there are empty frame in memory, load the designated page
			if(possible_empty!=-1){
				pcb_current->pagetable[pcb_current->currentPage]=loadPage(pcb_current->pid, pcb_current->currentPage);
				updateLRU(possible_empty/3);
				continue;
			}
			// other wise, replace following LRU
			int targetPage = leastRecentUsed();
			//printf("the least resent used one is %d\n", targetPage);
			pcb_current->pagetable[pcb_current->currentPage]=replacePage(pcb_current->pid, pcb_current->currentPage,targetPage*3);
			updateLRU(targetPage);
			//showMemory();
			continue;
		}
		// regular run
		else{
			page_index += pcb_current->currentFrame;
			char * line = mem_get_value_by_index(page_index);
			if(strcmp(line,"empty")!=0){
				parseInput(line);
			}
						
		}
		pcb_current->currentFrame++;
		if(pcb_current->currentFrame==3){
			pcb_current->currentFrame=0;
			pcb_current->currentPage++;
		}
	}
	// free the dynamically allocated memory
	free(pcb_current);
	mem_reset();
	head = NULL;
	tail = NULL;
	return errCode;
}
int replacePage(char *pid, int numPage, int target){	
	char pagename[30] = {};
	strcat(pagename, "backingstore/");;
	strcat(pagename, pid);
	strcat(pagename, "-");
	char npage[3];
	sprintf(npage,"%d",numPage);
	strcat(pagename, npage);
	FILE *pg = fopen(pagename, "rt");
	char line[1000];
	fgets(line,999,pg);
	int start_index = -1;
	printf("Page fault! Victim page contents:\n");
	for(int i=0;i<3;i++){
		//printf("replacing line is %s", line);
		if(feof(pg)||strcmp(line,"\n")==0){
			//到头了，没到3行，写入表示空行的
			for(int j=i;j<3;j++){
				printf("%s", mem_get_value_by_index(target+j));
				mem_set_value(target+j, "empty");
			}
			break;
		}
		//printf("%s",line);
		int length = strlen(line);
		if(line[length-1]!='\n'){
			line[length] = '\n';
			line[length+1] = '\0';
		}
		printf("%s", mem_get_value_by_index(target+i));
		int mem_index = mem_set_value(target+i, line);
		if(i==0){
			start_index=mem_index;
		}
		memset(line, 0, sizeof(line));
		fgets(line,999,pg);
	}
	fclose(pg);
	printf("End of victim page contents.\n");
	return start_index;
}

int loadPage(char *pid, int numPage){	
	char pagename[30] = {};
	strcat(pagename, "backingstore/");;
	strcat(pagename, pid);
	strcat(pagename, "-");
	char npage[3];
	sprintf(npage,"%d",numPage);
	strcat(pagename, npage);
	FILE *pg = fopen(pagename, "rt");
	char line[1000];
	fgets(line,999,pg);
	int start_index = -1;
	for(int i=0;i<3;i++){
		if(feof(pg)){
			//到头了，没到3行，写入表示空行的
			for(int j=i;j<3;j++){
				mem_set_value(-1, "empty");
			}
			break;
		}
		//printf("%s",line);
		int length = strlen(line);
		//printf("line length is %d\n",length);
		if(line[length-1]!='\n'){
			line[length] = '\n';
			line[length+1] = '\0';
		}
		int mem_index = mem_set_value(-1, line);
		if(i==0){
			start_index=mem_index;
		}
		memset(line, 0, sizeof(line));
		fgets(line,999,pg);
	}
	fclose(pg);
	return start_index;
}

int exec(char* command_args[], int args_size){
	pcb_t *(pcbQueue[args_size-2]);
	// when only one scirpt is given, the call
	// run directly, because whatever scheduling
	// policy given, the result will be the same
	int errCode = 0;
	if(args_size == 3){
		return run(command_args[1]);
	}
	printf("Frame Store Size = %d; Variable Store Size = %d\n", FRAMESIZE, VARMEMSIZE);
	// dynamically allocate each pcb
	for(int i=0;i<args_size-2;i++){
		pcbQueue[i]=(pcb_t *)malloc(sizeof(pcb_t));
	}
	// save script pages into backingstore
	for(int i=0;i<args_size-2;i++){
		char line[1000];
		FILE *p=fopen(command_args[i+1],"rt");
		if(p==NULL){
			return badcommandFileDoesNotExist();
		}
		fgets(line,999,p);
		char new_pid[4];
		sprintf(new_pid, "%d",pid_count);
		strcpy(pcbQueue[i]->pid, new_pid);
		int page_count = 0;
		int line_count = 0;
		int write_page_done = -1;
		while(write_page_done == -1){
			char pagename[30]={};
			strcat(pagename,"backingstore/");
			strcat(pagename, new_pid);
			strcat(pagename, "-");
			char char_page_count[3];
			sprintf(char_page_count, "%d", page_count);
			strcat(pagename, char_page_count);
			FILE *write_page = fopen(pagename, "w");
			for(int i=0;i<3;i++){
				int length = strlen(line);
				if(line[length-1]!='\n'){
					line[length] = '\n';
					line[length+1] = '\0';
				}
				fprintf(write_page,"%s",line);
				memset(line, 0, sizeof(line));
				line_count++;
				if(feof(p)){
					write_page_done=1;
					break;
				}
				fgets(line,999,p);
			}
			fclose(write_page);
			page_count++;	
		}
		fclose(p);
		pcbQueue[i]->length = line_count;
		pcbQueue[i]->pc=0;
		pid_count++;
		pcbQueue[i]->totalPage = page_count;
		pcbQueue[i]->currentPage=0;
		pcbQueue[i]->currentFrame=0;
		pcbQueue[i]->pagetable=malloc(pcbQueue[i]->totalPage*sizeof(int));
		for(int j=0;j<pcbQueue[i]->totalPage;j++){
			pcbQueue[i]->pagetable[j]=-1;
		}
		// save first two pages into shell memory
		if(pcbQueue[i]->length<=3){
			pcbQueue[i]->pagetable[0]=loadPage(pcbQueue[i]->pid, 0);
			updateLRU(pcbQueue[i]->pagetable[0]);
		}
		else{
			pcbQueue[i]->pagetable[0]=loadPage(pcbQueue[i]->pid, 0);
			updateLRU(pcbQueue[i]->pagetable[0]/3);
			pcbQueue[i]->pagetable[1]=loadPage(pcbQueue[i]->pid, 1);
			updateLRU(pcbQueue[i]->pagetable[1]/3);
		}
	}

	// 看pagetable
	/*
	for(int i=0;i<args_size-2;i++){
		for(int j=0;j<pcbQueue[i]->totalPage;j++){
			printf("%d ", pcbQueue[i]->pagetable[j]);
		}
		printf("\n");
	}
	*/
	//showLRU();
	//showMemory();
	// RR scheduling policy
	if(strcmp(command_args[args_size-1],"RR")==0){
		int rr = 2;
		int completed=0;
		// set up the readyQueue
		head = pcbQueue[0];
		tail = pcbQueue[args_size-3];
		for(int i=0;i<args_size-3;i++){
			pcbQueue[i]->next = pcbQueue[i+1];
		}
		// execute
		while(1){
			//showLRU();
			// all process finished
			if(completed == args_size-2){
				break;
			}
			// remove the finished process from the readyQueue
			if(head->pc==head->length){
				//printf("here inside remove completed ones, pid=%s\n",head->pid);
				rr=2;
				pcb_t *temp = head;
				head = head->next;
				temp->next=NULL;
				completed++;
				//printf("new head pid=%s\n",head->pid);
				continue;
			}
			// handle page fault
			if(head->pagetable[head->currentPage]==-1){
				int possible_empty = empty_spot();
				//printf("current head is: %s\n",head->pid);
				// found empty spot
				if(possible_empty!=-1){
					head->pagetable[head->currentPage]=loadPage(head->pid, head->currentPage);
					updateLRU(possible_empty/3);
				}
				// no empty spot
				else{
					int targetPage = leastRecentUsed();
					//printf("the least resent used one is %d\n", targetPage);
					head->pagetable[head->currentPage]=replacePage(head->pid, head->currentPage, targetPage*3);
					updateLRU(targetPage);
				}
				// move the current process to the end of the ready queue
				if(args_size-2-completed>1){
					pcb_t *temp = head;
					head = head->next;
					tail->next = temp;
					tail = tail->next;
					tail->next = NULL;
				}
				rr=2;
				continue;
			}
			int memIndex = head->pagetable[head->currentPage] + head->currentFrame;
			//printf("current page is %d, current frame is %d, memIndex is %d\n",head->pagetable[head->currentPage], head->currentFrame, memIndex);
			char *line = mem_get_value_by_index(memIndex);
			//printf("the line content to execute: %s",line);
			if(strcmp(line, "empty")==0){
				printf("reads empty\n");
				continue;
			}
			errCode = parseInput(line);
			updateLRU(head->pagetable[head->currentPage]/3);
			head->currentFrame++;
			if(head->currentFrame==3){
				head->currentFrame=0;
				head->currentPage++;
			}
			head->pc++;

			rr--;
			// reset rr, and the head process shall
			// be moved to tail
			if(rr==0){
				rr=2;
				if(args_size-2-completed>1){
					pcb_t *temp = head;
					head = head->next;
					tail->next = temp;
					tail = tail->next;
					tail->next = NULL;
				}
			}
		}
	}
	printf("herewow!\n");
	// clean mysh memory and free these dym-allocated structs
	for(int i=0;i<args_size-2; i++){
		free(pcbQueue[i]->pagetable);
		free(pcbQueue[i]);
	}
	//showMemory();
	mem_reset();
	head=NULL;
	tail=NULL;
	return errCode;
}

int my_ls(){
	int errCode = system("ls | sort");
	return errCode;
}

int echo(char* var){
	if(var[0] == '$'){
		var++;
		printf("%s\n", var_get_value(var)); 
	}else{
		printf("%s\n", var); 
	}
	return 0; 
}