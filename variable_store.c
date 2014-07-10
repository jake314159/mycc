#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "compiler.h"
#include "variable_store.h"

typedef struct VAR_PAIR{
	char *name;
	char *location;
} VAR_PAIR;

typedef struct STORE_LOCAL_LL{
	struct VAR_PAIR *pair;
	struct STORE_LOCAL_LL *next;
} STORE_LOCAL_LL;

VAR_PAIR* get_last_pair(VAR_STORE *store);
VAR_PAIR* get_var_pair(VAR_STORE *store,  char* name);
VAR_PAIR* get_var_pair_for_location(VAR_STORE *store,  char* location);

int stack_used = 0;

char *register_name[7] =          {"%r10d", "%r11d", "%ebx", "%r12d", "%r13d", "%r14d", "%r15d"};
bool register_used[7] = 		  {true,    false,   false,  false,   false,   false,   false  };
char *register_save_location[7] = {NULL,    NULL,    NULL,   NULL,    NULL,    NULL,    NULL   };

int compare_string(char* string1, char* string2)
{
	return strcmp(string1, string2);
}

char* get_stack_space(int amount) //TODO amount not used
{
	if(amount<=0) return NULL;
	printf("    subq    $4, %rsp\n");
	stack_used += 4;
	char *c = malloc(sizeof(char)*20);
	sprintf(c, "-%d(%%rbp)", stack_used);
	return c;
}

char* get_free_register()
{
	int i = 0;
	//The first 2 don't need to be saved so try to use these first
	for(;i<2; i++) {
		if(!register_used[i]) {
			register_used[i] = true;
			return register_name[i];
		}
	}

	// These regs need to be put back to their original values so lets save it
	for(;i<7;i++) {
		if(!register_used[i]) {
			register_used[i] = true;
			if(register_save_location[i] == NULL) {
				register_save_location[i] = get_stack_space(4);
				move_values(register_name[i], register_save_location[i]);
			}
			return register_name[i];
		}
	}
	return NULL; // no free registers
}

void free_register(char *reg_to_free)
{
	int i = 0;
	for(;i<7;i++) {
		if(compare_string(reg_to_free, register_name[i]) == 0) {
			register_used[i] = false;
			break;
		}
	}
}

void init_local_store(VAR_STORE *store)
{
	if(store == NULL) return;

	store->type = 'l'; //Local
	store->contents = NULL; //starts empty
}

VAR_PAIR* get_var_pair(VAR_STORE *store,  char* name)
{
	if(store == NULL) return NULL;

	STORE_LOCAL_LL *ll;
	ll = (STORE_LOCAL_LL*)store->contents;
	while(ll != NULL) {
		if(compare_string(ll->pair->name, name) == 0) {
			return ll->pair;
		} else {
			ll = ll->next;
		}
	}
	return NULL;
}

VAR_PAIR* get_var_pair_for_location(VAR_STORE *store,  char* location)
{
	if(store == NULL) return NULL;

	STORE_LOCAL_LL *ll;
	ll = (STORE_LOCAL_LL*)store->contents;
	while(ll != NULL) {
		if(compare_string(ll->pair->location, location) == 0) {
			return ll->pair;
		} else {
			ll = ll->next;
		}
	}
	return NULL;
}

//and make a new one on the end (which is the one which is returned)
VAR_PAIR* get_last_pair(VAR_STORE *store)
{
	if(store == NULL) return NULL;

	STORE_LOCAL_LL *ll;
	ll = (STORE_LOCAL_LL*)(store->contents);
	if(ll == NULL) {
		store->contents = malloc(sizeof(STORE_LOCAL_LL));
		((STORE_LOCAL_LL*)store->contents)->pair = malloc(sizeof(VAR_PAIR));
		((STORE_LOCAL_LL*)store->contents)->next = NULL;
		return ((STORE_LOCAL_LL*)store->contents)->pair;
	}

	while(ll->next != NULL) {
		ll = ll->next;
	}
	ll->next = malloc(sizeof(STORE_LOCAL_LL));
	ll->next->pair = malloc(sizeof(VAR_PAIR));
	ll->next->next = NULL;

	return ll->next->pair;
}

void add_local_var(VAR_STORE *store, char* name, char* location)
{
	if(store == NULL) return;
	char *temp = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(temp, name);
	name = temp;
	
	temp = malloc(sizeof(char) * (strlen(location) + 1));
	strcpy(temp, location);
	location = temp;

	VAR_PAIR *pair;
	pair = get_var_pair(store, name);
	if(pair == NULL) {
		pair = get_last_pair(store);
		pair->name = name;
		pair->location = location;
	} else {
		free(pair->location);
		pair->location = location;
	}
}

char* get_local_var_location(VAR_STORE *store, char *name)
{
	if(store == NULL) return NULL;

	VAR_PAIR *pair = get_var_pair(store, name);
	if(pair == NULL) {
		//assume it is a global variable
		char *c = malloc(sizeof(char)*(strlen(name)+20));
		sprintf(c, "%s(%%rip)", name);
		return c;
	} else {
		return pair->location;
	}
}

void set_local_var(VAR_STORE *store, char* name, char* location)
{
	if(store == NULL) return;

	VAR_PAIR *pair = get_var_pair(store, name);
	if(pair != NULL) {
		move_values(location, pair->location);
	} else {
		//it's a global variable
		char *c = malloc(sizeof(char) *(strlen(name)+20));
		sprintf(c, "%s(%%rip)", name);
		move_values(location, c);
	}
}

void create_local_var(VAR_STORE *store, char* name)
{
	if(store == NULL) return;
	char *temp = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(temp, name);

	VAR_PAIR *pair = get_last_pair(store);

	pair->name = temp;
	pair->location = malloc(sizeof(char) * 20);

	stack_used += 4;
	printf("    subq    $4, %%rsp\n"); //Add some space to the stack
	sprintf(pair->location, "-%d(%%rbp)", stack_used);
}

void free_location(VAR_STORE *store, char *location)
{
	VAR_PAIR *pair = get_var_pair_for_location(store, location);
	if(pair != NULL) {
		//We need to move this variable somewhere else
		stack_used += 4;
		printf("    subq    $4, %%rsp\n"); //Add some space to the stack
		printf("    movl    %s, -%d(%%rbp)\n", pair->location, stack_used);
		sprintf(pair->location, "-%d(%%rbp)", stack_used);
	}
}

void empty_stack_of_local_vars()
{
	int i=2;
	for(;i<7;i++) {
		if(register_save_location[i] != NULL) {
			register_used[i] = false;
			move_values(register_save_location[i], register_name[i]);
			register_save_location[i] = NULL;
		}
	}

	if(stack_used > 0) {
		printf("    addq    $%d, %%rsp\n", stack_used);
		stack_used = 0;
	}
}

