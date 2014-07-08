#ifndef ___STOP_MULTI_IMPORT_PARSE_TREE_____
#define ___STOP_MULTI_IMPORT_PARSE_TREE_____ 1

enum tree_type {
	NODE_INT, NODE_STRING, NODE_FLOAT, NODE_BODY, NODE_PARAMITER_DEF, NODE_PARAMITER_CHAIN,
	NODE_FUNCTION_DEF, NODE_FUNCTION_TYPE_NAME_PAIR, NODE_FUNCTION_PARAM_BODY_PAIR,
	NODE_MAIN_EXTENDED, NODE_VAR, NODE_VAR_ASSIGN, NODE_VAR_DEF,
	NODE_ADD, NODE_SUB,
	NODE_FUNCTION_CALL, NODE_FUNCTION_ARG_CHAIN,
	NODE_RETURN
};

typedef struct ptree {
   enum tree_type type;
   union {
     struct {struct ptree *left, *right;} a_parent;
     int a_int;
     float a_float;
     char *a_string;
   } body;
 } ptree;

ptree* make_node_int(int type);
ptree* make_node_string(char *type);
ptree* make_node_float(float type);
ptree* make_node_body(ptree *line, ptree *remaining);
ptree* make_node_paramiter_def(ptree *other_params, ptree *type, ptree *name);
ptree* make_node_function_def(ptree *type, ptree *name, ptree *params, ptree *body);
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

ptree* make_var_def(ptree *varType, ptree *name);

void free_tree(ptree *root);
ptree* getTreeRoot();

#endif

