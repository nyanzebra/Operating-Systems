#ifndef _CHILDTABLE_H_
#define _CHILDTABLE_H_

#include <pid.h>

struct childtable {
	pid_t pid;
	volatile int finished;
	int exit_code;
	struct childtable* next;
};

#endif
