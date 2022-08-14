// Haochen Liu
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


struct memory_struct{
	char *var;
	char *value;
};

struct memory_struct shellmemory[FRAMESIZE+VARMEMSIZE];

int LRU[FRAMESIZE/3];

// Shell memory function
void mem_init(){
	int i;
	for(i=0; i<FRAMESIZE+VARMEMSIZE; i++){
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
	for(i=0;i<FRAMESIZE/3;i++){
		LRU[i]=-1;
	}
}
void updateLRU(int index){
	for(int i=0;i<FRAMESIZE/3;i++){
		if(LRU[i]!=-1 && i!=index){
			LRU[i]++;
		}
		if(i==index){
			LRU[i]=0;
		}
	}
}
int leastRecentUsed(){
	int result=0;
	int age=LRU[0];
	for(int i=1;i<FRAMESIZE/3;i++){
		if(LRU[i]>age){
			result=i;
			age=LRU[i];
		}
	}
	return result;
}
void showLRU(){
	for(int i=0;i<FRAMESIZE/3;i++){
		printf("%d ", LRU[i]);
	}
	printf("\n");
}
void showMemory(){
	for(int i=0;i<FRAMESIZE;i++){
		printf("%s",shellmemory[i].var);
		printf("  ");
		printf("%s\n",shellmemory[i].value);
	}
}

void showVariable(){
	for(int i=FRAMESIZE;i<FRAMESIZE+VARMEMSIZE;i++){
		printf("%s",shellmemory[i].var);
		printf("  ");
		printf("%s\n",shellmemory[i].value);
	}
}
void mem_reset(){
	int i;
	for(i=0; i<FRAMESIZE; i++){
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
	for(i=0;i<FRAMESIZE/3;i++){
		LRU[i]=-1;
	}
}
void var_reset(){
	for(int i=FRAMESIZE;i<FRAMESIZE+VARMEMSIZE;i++){
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}
// set code storage in memory - by index, not by var
int mem_set_value(int index, char *value_in) {
	int i;
	int out = 0;
	if(index>=0){
		shellmemory[index].value = strdup(value_in);
		return index;
	}
	//Value does not exist, need to find a free spot.
	if(index == -1){
		for (i=0; i<FRAMESIZE; i++){
			if (strcmp(shellmemory[i].value, "none") == 0){
				shellmemory[i].value = strdup(value_in);
				return out;
			}
			out++; 
		}
	}
	return -1;
}
//get frame value based on input key
char *mem_get_value(char *var_in) {
	int i;
	for (i=0; i<FRAMESIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return strdup(shellmemory[i].value);
		} 
	}
	return "pagefault";

}
char *mem_get_value_by_index(int index) {
	if(index >= FRAMESIZE){
		return "out of bound";
	}
	return shellmemory[index].value;
}
// Set frame key value pair
int var_set_value(char *var_in, char *value_in) {

	int i;
	int out = 0;
	for (i=FRAMESIZE; i<FRAMESIZE+VARMEMSIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return 0;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=FRAMESIZE; i<FRAMESIZE+VARMEMSIZE; i++){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			return out;
		}
		out++; 
	}
	return -1;
}
//get frame value based on input key
char *var_get_value(char *var_in) {
	int i;
	for (i=FRAMESIZE; i<FRAMESIZE+VARMEMSIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return strdup(shellmemory[i].value);
		} 
	}
	return "Variable does not exist";

}
// reset the designated spot in memory to none 
void mem_clean(char *var_in){
	int i;
	for (i=0; i<FRAMESIZE+VARMEMSIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].var = "none";
			memset(shellmemory[i].value, 0, sizeof(shellmemory[i].value));
			shellmemory[i].value = "none";
		} 
	}
}

// to find the index of starting empty spot, -1 means not found
int empty_spot(){
	for(int i=0;i<FRAMESIZE;i++){
		if(strcmp(shellmemory[i].value,"none")==0){
			if(strcmp(shellmemory[i+1].value,"none")==0
			&& strcmp(shellmemory[i+2].value,"none")==0){
				return i;
			}
		}
	}
	return -1;
}