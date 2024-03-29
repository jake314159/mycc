#ifndef ___STOP_MULTI_IMPORT_PARSE_TREE_____
#define ___STOP_MULTI_IMPORT_PARSE_TREE_____ 1
#include <stdbool.h>

enum tree_type {
	NODE_INT, NODE_STRING, NODE_FLOAT, NODE_BODY, NODE_PARAMITER_DEF, NODE_PARAMITER_CHAIN, NODE_PARAMITER_DEF_POINTER,
	NODE_STRING_CONST,
	NODE_FUNCTION_DEF, NODE_FUNCTION_TYPE_NAME_PAIR, NODE_FUNCTION_PARAM_BODY_PAIR,
	NODE_MAIN_EXTENDED, NODE_VAR, NODE_VAR_ASSIGN, NODE_VAR_DEF, NODE_VAR_GLOBAL_DEF,
	NODE_ADD, NODE_SUB, NODE_U_MINUS, NODE_MULT, NODE_DIV,
	NODE_FUNCTION_CALL, NODE_FUNCTION_ARG_CHAIN,
	NODE_IF, NODE_FUNCTION_TYPE_POINTER_NAME_PAIR, NODE_VAR_TO_POINTER,
	NODE_VAR_POINTER, NODE_VAR_DEF_POINTER, NODE_VAR_GLOBAL_DEF_POINTER, NODE_VAR_ASSIGN_POINTER,
	NODE_COMP_GT, NODE_COMP_LT, NODE_COMP_EQ, NODE_COMP_NEQ, NODE_COMP_GTEQ, NODE_COMP_LTEQ,
	NODE_RETURN
};

typedef struct ptree {
   enum tree_type type;
   union {
     struct {struct ptree *left, *right, *extra;} a_parent;
     int a_int;
     float a_float;
     char *a_string;
   } body;
 } ptree;

ptree* make_node_int(int type);
ptree* make_node_string(char *type);
ptree* make_node_float(float type);
ptree* make_node_body(ptree *line, ptree *remaining);
ptree* make_node_paramiter_def(ptree *other_params, ptree *type, ptree *name, bool pointer);
ptree* make_node_function_def(ptree *type, bool pointer, ptree *name, ptree *params, ptree *body);
ptree* make_main_extended(ptree *ext, ptree *line);
void printTree(ptree *root, int depth);
void print_last_tree();
ptree* make_return_node(ptree *returnValue);
ptree* make_function_call(ptree *fun_name, ptree *args);
ptree* make_function_args(ptree *moreArgs, ptree *argValue);
ptree* make_node_var(ptree *string_node); //Converts a string node to a var node
ptree* make_var_assign(ptree *varName, ptree *value);
ptree* make_node_add(ptree *left, ptree *right);
ptree* make_node_sub(ptree *left, ptree *right);
ptree* make_node_mult(ptree *left, ptree *right);
ptree* make_node_unary_minus(ptree *value);
ptree* make_node_global_var_def(ptree *varType, ptree *name, ptree *default_value);
ptree* make_node_string_const(char *type);
ptree* make_node_if(ptree* value, ptree* true_body, ptree* false_body);
ptree* make_node_comp(enum tree_type type, ptree *left, ptree *right);
ptree* make_node_var_pointer(ptree *string_node);
ptree* make_var_def_pointer(ptree *varType, ptree *name);
ptree* make_node_global_var_def_pointer(ptree *varType, ptree *name, ptree *default_value);
ptree* make_var_assign_pointer(ptree *varName, ptree *value);
ptree* make_node_var_to_pointer(ptree *string_node);
ptree* make_node_div(ptree *left, ptree *right);

ptree* make_var_def(ptree *varType, ptree *name);

void free_tree(ptree *root);
ptree* getTreeRoot();

#endif

