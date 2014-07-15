#include <stdbool.h>
#include "parse_tree.h"
#include "type_manager.h"
#include "store.h"

typedef struct PARAM_ENTRY {
    int param_number;
    TYPE_DEF type;
    struct PARAM_ENTRY *next;
} PARAM_ENTRY;

typedef struct FUNC_ENTRY {
    TYPE_DEF type;
    char *name;
    PARAM_ENTRY *param;
} FUNC_ENTRY;

void init_funk_store(VAR_STORE *store);
bool add_function_to_store(VAR_STORE *store, ptree *tree);
FUNC_ENTRY* get_function_entry(VAR_STORE *store, char *name);

