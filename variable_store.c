#include <stdlib.h>
#include <stdio.h>
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

int compare_string(char* string1, char* string2)
{
    int i = 0;
    while(string1[i] != '\0' && string2[i] != '\0') {
        if(string1[i] != string2[i]) {
            if(string1[i] < string2[i]) {
                return 1;
            } else {
                return -1;
            }
        }
        i++;
    }
    return 0; //0 for equal
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
		return NULL;
	} else {
		return pair->location;
	}
}
void set_local_var(VAR_STORE *store, char* name, char* location)
{
	if(store == NULL) return;

	VAR_PAIR *pair = get_var_pair(store, name);
	if(pair != NULL) {
		printf("    movq    %s, %s\n", location, pair->location);
	}
}

void create_local_var(VAR_STORE *store, char* name)
{
	if(store == NULL) return;
	char *temp = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(temp, name);
	name = temp;

	VAR_PAIR *pair = get_last_pair(store);

	pair->name = name;
	pair->location = malloc(sizeof(char) * 20);

	stack_used += 8;
	printf("    subq    $8, %%rsp\n"); //Add some space to the stack
	sprintf(pair->location, "-%d(%%rbp)", stack_used);
}

void free_location(VAR_STORE *store, char *location)
{
	VAR_PAIR *pair = get_var_pair_for_location(store, location);
	if(pair != NULL) {
		//We need to move this variable somewhere else
		stack_used += 8;
		printf("    subq    $8, %%rsp\n"); //Add some space to the stack
		printf("    movq    %s, -%d(%%rbp)\n", pair->location, stack_used);
		sprintf(pair->location, "-%d(%%rbp)", stack_used);
	}
}

void empty_stack_of_local_vars()
{
	if(stack_used > 0) {
		printf("    addq    $%d, %%rsp\n", stack_used);
		stack_used = 0;
	}
}

