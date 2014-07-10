#include "stdlib.h"
#include <stdio.h>
#include "parse_tree.h"

ptree *tree_root = NULL;

void print_last_tree()
{
	printTree(tree_root, 0);
}

void printTree(ptree *root, int depth)
{
	int i;
	for(i=0; i<depth; i++) {
		printf("--");
	}

	if(root == NULL) {
		printf("NULL\n");
		return;
	}

	switch(root->type) {
		case NODE_INT:
			printf("Int %i\n", root->body.a_int);
			break;
		case NODE_STRING:
			printf("String '%s'\n", root->body.a_string);
			break;
		case NODE_STRING_CONST:
			printf("String constant '%s'\n", root->body.a_string);
			break;
		case NODE_VAR:
			printf("Variable '%s'\n", root->body.a_string);
			break;
		case NODE_FLOAT:
			printf("Float %f\n", root->body.a_float);
			break;
		case NODE_MAIN_EXTENDED:
			printf("Main ext\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_FUNCTION_DEF:
			printf("Function def\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_FUNCTION_TYPE_NAME_PAIR:
			printf("Function type-name pair\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_FUNCTION_PARAM_BODY_PAIR:
			printf("Function param-body pair\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_PARAMITER_DEF:
			printf("Paramiter def\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_PARAMITER_CHAIN:
			printf("Paramiter chain\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_RETURN:
			printf("Return\n");
			printTree(root->body.a_parent.left, depth+1);
			break;
		case NODE_FUNCTION_CALL:
			printf("Function call\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
		case NODE_FUNCTION_ARG_CHAIN:
			printf("Arg chain\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_VAR_ASSIGN:
			printf("Variable assignment\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_VAR_DEF:
			printf("Variable define\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_VAR_GLOBAL_DEF:
			printf("Global var\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_ADD:
			printf("[+]\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_SUB:
			printf("[-]\n");
			printTree(root->body.a_parent.left, depth+1);
			printTree(root->body.a_parent.right, depth+1);
			break;
		case NODE_U_MINUS:
			printf("[U-]\n");
			printTree(root->body.a_parent.left, depth+1);
			break;
		default:
			printf("Unknown %d\n", root->type);
			break;
	}
}

ptree* make_node_string(char *type)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_STRING;
	tree->body.a_string = type;
	return tree;
}

ptree* make_node_string_const(char *type)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_STRING_CONST;
	tree->body.a_string = type;
	return tree;
}

ptree* make_node_int(int type)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_INT;
	tree->body.a_int = type;
	return tree;
}

ptree* make_node_float(float type)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_FLOAT;
	tree->body.a_float = type;
	return tree;
}

ptree* make_node_body(ptree *line, ptree *remaining)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_BODY;
	tree->body.a_parent.left = line;
	tree->body.a_parent.right = remaining;
	return tree;
}

ptree* make_node_paramiter_def(ptree *other_params, ptree *type, ptree *name)
{
	if(other_params == NULL) {
		ptree *tree = malloc(sizeof(ptree));
		tree->type = NODE_PARAMITER_DEF;
		tree->body.a_parent.left = type;
		tree->body.a_parent.right = name;
		return tree;
	} else {
		ptree *tree = malloc(sizeof(ptree));
		tree->type = NODE_PARAMITER_CHAIN;
		tree->body.a_parent.right = make_node_paramiter_def(NULL, type, name);
		tree->body.a_parent.left = other_params;
		return tree;
	}
}

ptree* make_node_function_def(ptree *type, ptree *name, ptree *params, ptree *body)
{
	ptree *def = malloc(sizeof(ptree));
	ptree *name_pair = malloc(sizeof(ptree));
	ptree *param_pair = malloc(sizeof(ptree));

	def->type = NODE_FUNCTION_DEF;
	def->body.a_parent.left = name_pair;
	def->body.a_parent.right = param_pair;

	name_pair->type = NODE_FUNCTION_TYPE_NAME_PAIR;
	name_pair->body.a_parent.left = type;
	name_pair->body.a_parent.right = name;

	param_pair->type = NODE_FUNCTION_PARAM_BODY_PAIR;
	param_pair->body.a_parent.left = params;
	param_pair->body.a_parent.right = body;

	tree_root = def;

	return def;
}

ptree* make_main_extended(ptree *ext, ptree *line)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_MAIN_EXTENDED;
	tree->body.a_parent.left = ext;
	tree->body.a_parent.right = line;
	tree_root = tree;
	return tree;
}

ptree* make_return_node(ptree *returnValue)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_RETURN;
	tree->body.a_parent.left = returnValue;
	tree->body.a_parent.right = NULL;
	return tree;
}

ptree* make_function_call(ptree *fun_name, ptree *args)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_FUNCTION_CALL;
	tree->body.a_parent.left = fun_name;
	tree->body.a_parent.right = args;
	return tree;
}

ptree* make_function_args(ptree *moreArgs, ptree *argValue)
{
	ptree *tree = malloc(sizeof(ptree));
	tree->type = NODE_FUNCTION_ARG_CHAIN;
	tree->body.a_parent.left = moreArgs;
	tree->body.a_parent.right = argValue;
	return tree;
}

void free_tree(ptree *root)
{
	if(root == NULL) return;
	switch(root->type) {
		case NODE_INT:
		case NODE_STRING:
		case NODE_STRING_CONST:
		case NODE_FLOAT:
			free(root);
			break;
		default:
			free_tree(root->body.a_parent.left);
			free_tree(root->body.a_parent.right);
			free(root);
			break;
	}
}

ptree* getTreeRoot()
{
	return tree_root;
}

ptree* make_node_var(ptree *string_node)
{
	string_node->type = NODE_VAR;
	return string_node;
}

ptree* make_var_assign(ptree *varName, ptree *value)
{
	ptree *node = malloc(sizeof(ptree));
	node->type = NODE_VAR_ASSIGN;
	node->body.a_parent.left = varName;
	node->body.a_parent.right = value;
	return node;
}

ptree* make_var_def(ptree *varType, ptree *name)
{
	ptree *node = malloc(sizeof(ptree));
	node->type = NODE_VAR_DEF;
	node->body.a_parent.left = varType;
	node->body.a_parent.right = name;
	return node;
}

ptree* make_node_add(ptree *left, ptree *right)
{
	ptree *node = malloc(sizeof(ptree));
	node->type = NODE_ADD;
	node->body.a_parent.left = left;
	node->body.a_parent.right = right;
	return node;
}

ptree* make_node_sub(ptree *left, ptree *right)
{
	ptree *node = malloc(sizeof(ptree));
	node->type = NODE_SUB;
	node->body.a_parent.left = left;
	node->body.a_parent.right = right;
	return node;
}

ptree* make_node_unary_minus(ptree *value)
{
	ptree *node = malloc(sizeof(ptree));
	node->type = NODE_U_MINUS;
	node->body.a_parent.left = value;
	node->body.a_parent.right = NULL;
	return node;
}

ptree* make_node_global_var_def(ptree *varType, ptree *name, ptree *default_value)
{
	ptree *def_pair = malloc(sizeof(ptree));
	def_pair->type = NODE_VAR_DEF;
	def_pair->body.a_parent.left = varType;
	def_pair->body.a_parent.right = name;

	ptree *node = malloc(sizeof(ptree));
	node->type = NODE_VAR_GLOBAL_DEF;
	node->body.a_parent.left = def_pair;
	node->body.a_parent.right = default_value;
	return node;
}

