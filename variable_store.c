#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "compiler.h"
#include "variable_store.h"

typedef struct VAR_PAIR{
	char *name;
	char *location;
	TYPE_DEF type;
} VAR_PAIR;

typedef struct STORE_LOCAL_LL{
	struct VAR_PAIR *pair;
	struct STORE_LOCAL_LL *next;
} STORE_LOCAL_LL;

VAR_PAIR* get_last_pair(VAR_STORE *store);
VAR_PAIR* get_var_pair(VAR_STORE *store,  char* name);
VAR_PAIR* get_var_pair_for_location(VAR_STORE *store,  char* location);

int stack_used = 0;

char *register_name_8[7] =          {"%r10", "%r11", "%rbx", "%r12", "%r13", "%r14", "%r15"};
char *register_name_4[7] =          {"%r10d", "%r11d", "%ebx", "%r12d", "%r13d", "%r14d", "%r15d"};
char *register_name_2[7] =          {"%r10w", "%r11w", "%bx", "%r12w", "%r13w", "%r14w", "%r15w"};
char *register_name_1[7] =          {"%r10b", "%r11b", "%bl", "%r12b", "%r13b", "%r14b", "%r15b"};
bool register_used[7] = 		  {true,    false,   false,  false,   false,   false,   false  };
char *register_save_location[7] = {NULL,    NULL,    NULL,   NULL,    NULL,    NULL,    NULL   };

int compare_string(char* string1, char* string2)
{
	return strcmp(string1, string2);
}

char* get_stack_space(int amount)
{
	if(amount<=0) return NULL;
	printf("    subq    $%d, %%rsp\n", amount);
	stack_used += amount;
	char *c = malloc(sizeof(char)*20);
	sprintf(c, "-%d(%%rbp)", stack_used);
	return c;
}

char* get_free_register(int size)
{
	int i = 0;
	//The first 2 don't need to be saved so try to use these first
	for(;i<2; i++) {
		if(!register_used[i]) {
			register_used[i] = true;
			switch(size) {
				case 8:
					return register_name_8[i];
				case 2:
					return register_name_2[i];
				case 1:
					return register_name_1[i];
				default:
					return register_name_4[i];
			}
		}
	}

	// These regs need to be put back to their original values so lets save it
	for(;i<7;i++) {
		if(!register_used[i]) {
			register_used[i] = true;
			if(register_save_location[i] == NULL) {
				register_save_location[i] = get_stack_space(4);
				move_values(register_name_4[i], register_save_location[i], size);
			}
			return register_name_4[i]; //TODO depend on size 
		}
	}
	return NULL; // no free registers
}

void free_register(char *reg_to_free)
{
	int i = 0;
	for(;i<7;i++) {
		if(compare_string(reg_to_free, register_name_4[i]) == 0) {
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

void add_local_var(VAR_STORE *store, char* name, char* location, TYPE_DEF type)
{

    if(type.pointer) type.size = 8;

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
		pair->type = type;
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
        return NULL; //Can't find anything
	} else {
		return pair->location;
	}
}

int getRegisterSize(char *reg)
{
	if(reg[0] != '%') return 0; //It isn't a register!
	int length = strlen(reg);

	if(reg[2] >= '0' && reg[2] <= '9') {
		//It's one of the %r[8-15][dwb]?
		if(reg[length-1] == 'b') return 1;
		else if(reg[length-1] == 'w') return 2;
		else if(reg[length-1] == 'd') return 4;
		else                          return 8;
	} else if(reg[1] == 'r') {
		return 8;
	} else if(reg[1] == 'e') {
		return 4;
	} else if(reg[length-1] == 'l') {
		return 1;
	} else {
		return 2;
	}
}
//Not complete!!
char* convertReg(char *location, int targetSize)
{
	if(targetSize<=0 || targetSize>8) return NULL;
	char *returnReg = malloc(sizeof(char)*(strlen(location)+1));
	sprintf(returnReg,"%s", location);

	if(returnReg[1] == 'e') {
		switch(targetSize) {
			case 8:
				returnReg[1] = 'r';
				return returnReg;
				break;
		}
	}
	return returnReg;
}

bool set_local_var(VAR_STORE *store, char* name, char* location)
{
	if(store == NULL) return false;

	VAR_PAIR *pair = get_var_pair(store, name);
	if(pair != NULL) {
		int regLength = getRegisterSize(location);
		if(regLength == 0 || regLength == pair->type.size) {
			move_values(location, pair->location, pair->type.size);
		} else {
			//The register is the wrong size so we should try to convert it to the right size
			char *newLocation = convertReg(location, pair->type.size);
			move_values(location, newLocation, pair->type.size);
		}
        return true;
	} else {
        return false;
	}
}

bool set_local_var_pointer(VAR_STORE *store, char* name, char* location)
{
	if(store == NULL) return false;

	VAR_PAIR *pair = get_var_pair(store, name);
	if(pair != NULL) {
		printf("    movq    %s, %%rax\n", pair->location);
		printf("    movl    %s, (%%rax)\n", location);
        return false;
	} else {
        return false;
	}
}

void create_local_var(VAR_STORE *store, char* name, TYPE_DEF type)
{
	if(store == NULL) return;
	char *temp = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(temp, name);

	VAR_PAIR *pair = get_last_pair(store);

	pair->name = temp;
	pair->location = malloc(sizeof(char) * 20);
	pair->type = type;

	stack_used += type.size;
	printf("    subq    $%d, %%rsp\n", type.size); //Add some space to the stack
	sprintf(pair->location, "-%d(%%rbp)", stack_used);
}

void free_location(VAR_STORE *store, char *location)
{
	bool freeLocation = false;
	VAR_PAIR *pair = get_var_pair_for_location(store, location);
	if(pair != NULL) {
		//We need to move this variable somewhere else
		int size = pair->type.size;
		char moveType;
		if(size == 8){
			moveType = 'q';
		} else {
			moveType = 'l';
		}
		stack_used += size;
		printf("    subq    $%d, %%rsp\n",size); //Add some space to the stack
		printf("    mov%c    %s, -%d(%%rbp)\n", moveType, pair->location, stack_used);
		sprintf(pair->location, "-%d(%%rbp)", stack_used);
		if(freeLocation) free(location);
	}
}

void empty_stack_of_local_vars()
{
	int i=2;
	for(;i<7;i++) {
		if(register_save_location[i] != NULL) {
			register_used[i] = false;
			move_values(register_save_location[i], register_name_4[i], 4); //TODO consider size to transfer
			register_save_location[i] = NULL;
		}
	}

	if(stack_used > 0) {
		printf("    addq    $%d, %%rsp\n", stack_used);
		stack_used = 0;
	}
}

int get_var_size_by_location(VAR_STORE *store, char *location)
{
	VAR_PAIR *pair = get_var_pair_for_location(store, location);
	if(pair == NULL) {
		//No idea so return an error code
		//TODO do this more properly. What if it's a register for example. Or global
		return -1;
	} else {
		return pair->type.size;
	}
}

