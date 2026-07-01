#include <stdio.h>
#include <string.h>
#include "history.h"

#define MAX_HISTORY 1000

static int history_count = 0;
static char *history[MAX_HISTORY];

void add_history(char *line){
    if(line == NULL)
        return;
    
    if(history_count < MAX_HISTORY)
        history[history_count++] = strdup(line);    
}

void print_history(void){
    for(int i= 0 ;i<history_count;i++){
        printf("%3d %s\n",i+1,history[i]);
    }
}