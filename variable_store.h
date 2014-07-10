

typedef struct {
	char type;
	void *contents;
} VAR_STORE;

void init_local_store(VAR_STORE *store);
void add_local_var(VAR_STORE *store, char* name, char* location);
char* get_local_var_location(VAR_STORE *store, char *name);
void set_local_var(VAR_STORE *store, char* name, char* location);
void create_local_var(VAR_STORE *store, char* name);

char* get_stack_space(int amount);
char* get_free_register();
void free_register(char *reg_to_free);
void free_location(VAR_STORE *store, char *location);
void empty_stack_of_local_vars();
