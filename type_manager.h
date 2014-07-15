#ifndef ___STOP_MULTI_IMPORT_TYPE_MANAGER_____
#define ___STOP_MULTI_IMPORT_TYPE_MANAGER_____ 1

#include <stdbool.h>

typedef struct {
	char *name;
	bool pointer;
	int size;
} TYPE_DEF;

void initTypeManager(); //Adds predefined types
void freeTypeManager(); //Frees memory used by the type manager
void defineType(TYPE_DEF typeDef);
TYPE_DEF* getType(char *typeName, bool pointer);

#endif

