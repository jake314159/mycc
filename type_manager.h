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

