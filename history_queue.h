/*
 * Header file for circular queue of HistoryListItems.
 */
#ifndef __HISTQUEUE_H__
#define __HISTQUEUE_H__
#include "command_line.h"


#define MAXHIST 10   // max number of commands in history list

/*
 * A struct to keep information one command in the history of 
 * command executed
 */
struct HistoryEntry
{              
    unsigned int cmd_num;
    struct CommandLine command; // command line for this process
};

// You can use "HistoryEntry" instead of "struct HistoryEntry"
typedef struct HistoryEntry HistoryEntry;


void add_to_history(struct CommandLine * command);
void print_history(int commandNum);
struct CommandLine * get_command(unsigned int cmd_num);
struct CommandLine * get_last_command();
void freeHistory();

#endif
