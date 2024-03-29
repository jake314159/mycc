#include "type_manager.h"
#include "store.h"

void init_local_store(VAR_STORE *store);
void add_local_var(VAR_STORE *store, char* name, char* location, TYPE_DEF type);
char* get_local_var_location(VAR_STORE *store, char *name);
// Return value is if it was successfull (ie. was there  a local var with that name)
bool set_local_var(VAR_STORE *store, char* name, char* location);
bool set_local_var_pointer(VAR_STORE *store, char* name, char* location);
void create_local_var(VAR_STORE *store, char* name, TYPE_DEF type);
int getRegisterSize(char *reg);

char* get_stack_space(int amount);
char* get_free_register(int size);
void free_register(char *reg_to_free);
void free_location(VAR_STORE *store, char *location);
void empty_stack_of_local_vars();
// Returns -1 for variable not found
int get_var_size_by_location(VAR_STORE *store, char *location);
