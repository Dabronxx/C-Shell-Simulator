/*
 * The Tiny Torero Shell (TTSH)
 *
 * Add your top-level comments here.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "history_queue.h"

static HistoryEntry history[MAXHIST];
static int next_index;
static int start_index;
static int list_size;
static int history_num;
static int lastEntry;

void add_to_history(struct CommandLine * command)
{
	// add the entry at the next spot
    
	history[next_index].cmd_num = history_num;
    lastEntry = next_index;
    
	history_num++;
    
    history[next_index].command.argCount = command->argCount;
    history[next_index].command.background = command->background;
    for (int i = 0; i < command->argCount; i++)
    {
        
        history[next_index].command.arguments[i] = malloc(strlen(command->arguments[i]));
        
        strncpy(history[next_index].command.arguments[i], command->arguments[i], strlen(command->arguments[i]));
        //printf("New argument: %s\n", history[next_index].command.arguments[i]);
        
    }
    // update where we'll put the next entry
	next_index = (next_index+1) % MAXHIST;
    
   // if we're at max capacity, we'll have to move the start index over to
	// account for overfilling.
	if (list_size >= MAXHIST)
    {
		start_index = (start_index+1) %  MAXHIST;
	}
	else
    {
		list_size++ ;
	}
    
}

void print_history(int commandNum)
{
    //printf("Test\n");
    if (commandNum == 0 || commandNum >= list_size)
    {
        for (int i = start_index, j = 0; j < list_size; j++, i = (i+1)%MAXHIST)
        {
            printf("%d\t", history[i].cmd_num);
            printCommand(&history[i].command);
            printf("\n");
        }
    }
    else
    {
        for (int i = list_size - commandNum, j = 0; j < commandNum; j++, i = (i+1)%MAXHIST)
        {
            printf("%d\t", history[i].cmd_num);
            printCommand(&history[i].command);
            printf("\n");
        }
    }
	
}

struct CommandLine * get_command(unsigned int cmd_num)
{
	for (int i = start_index, j = 0; j < list_size; j++, i = (i+1)%MAXHIST)
    {
        
		if (history[i].cmd_num == cmd_num)
        {
            
            return &history[i].command;
        }
				
	}
	return NULL;
}

struct CommandLine * get_last_command()
{
    if(list_size == 0)
    {
        return NULL;
    }
    
    return &history[lastEntry].command;
}

void freeHistory()
{
    for (int i = 0; i < list_size; i++)
    {
        freeCommand(&history[i].command);
        
    }
}
