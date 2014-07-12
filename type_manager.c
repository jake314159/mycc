#include <stdlib.h>
#include <string.h>
#include "type_manager.h"

#define START_TYPE_ARRAY_SIZE 30
#define POINTER_SIZE 8

TYPE_DEF *typeArray;
int number_of_types = 0;
int size_of_type_array;

void initTypeManager()
{
	size_of_type_array = START_TYPE_ARRAY_SIZE;
	typeArray = malloc(sizeof(TYPE_DEF) * size_of_type_array);

	
	typeArray[0].name    = "int";
	typeArray[0].pointer = false;
	typeArray[0].size    = 4;
	number_of_types++;

	typeArray[1].name    = "char";
	typeArray[1].pointer = false;
	typeArray[1].size    = 1;
	number_of_types++;
}

void freeTypeManager()
{
	free(typeArray);
}

void defineType(TYPE_DEF typeDef)
{
	if(number_of_types >= size_of_type_array) {
		return; //TODO make the array bigger
	}
	typeArray[number_of_types] = typeDef;
	number_of_types++;
}

TYPE_DEF* getType(char *typeName, bool pointer)
{
	int i = 0;
	int found_type = -1;
	for(i = 0; i<number_of_types; i++) {
		if(strcmp(typeArray[i].name, typeName) == 0) {
			found_type = i;
			break;
		}
	}
	if(found_type<0) return NULL; //Not found so error

	TYPE_DEF *def = malloc(sizeof(TYPE_DEF));
	def->name = typeArray[i].name;
	def->pointer = pointer;
	def->size = pointer ? POINTER_SIZE : typeArray[found_type].size;
	return def;
}

