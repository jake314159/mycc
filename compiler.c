#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "compiler.h"
#include "variable_store.h"

char *function_arg_locations_8[6]  = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
char *function_arg_locations[6]  = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};

int prep_function_args(ptree *tree, int argNumber);
char* to_value(ptree *tree);
char* get_paramiter_location(int n);

VAR_STORE *local_vars = NULL;

int functions_args_prep = 0; //Number of args we have prepared ready for a function call

int params_set = 0;
int string_const_number = 0;
int label_number = 0;

int function_return_size;

void create_head(ptree *tree)
{
	if(tree == NULL) return;

	switch(tree->type) {
		case NODE_INT:
		case NODE_STRING:
		case NODE_FLOAT:
			break;
		case NODE_STRING_CONST:
			printf("    .section    .rodata\n");
			printf(".LC%d:\n", string_const_number);
			printf("    .string %s\n\n", tree->body.a_string);
			free(tree->body.a_string);
			char *c = malloc(sizeof(char)*20);
			sprintf(c, "$.LC%d", string_const_number); 
			tree->body.a_string = c;
			string_const_number++;
			break;
		case NODE_FUNCTION_DEF:
			printf("    .globl   %s\n", tree->body.a_parent.left->body.a_parent.right->body.a_string);
			printf("    .type    %s, @function\n\n", tree->body.a_parent.left->body.a_parent.right->body.a_string);
			create_head(tree->body.a_parent.left);
			create_head(tree->body.a_parent.right);
			break;
		default:
			create_head(tree->body.a_parent.left);
			create_head(tree->body.a_parent.right);
			break;
	}
}

void move_values(char *from, char *too, int size)
{
	bool from_pointer = false, too_pointer = false, same = true;
	int i = 0;
	while(too[i] != '\0') {
		if(too[i] == '(') {
			too_pointer = true;
		}
		i++;
	}
	i=0;
	while(from[i] != '\0') {
		if(from[i] == '(') {
			from_pointer = true;
			break;
		}
		i++;
	}
	i = 0;
	while(from[i] != '\0' && too[i] != '\0') {
		if(from[i] != too[i]) {
			same = false;
			break;
		}
		i++;
	}

	char *move_cmd;
	//int regSize = getRegisterSize(too);
	if(size == 8) {
		move_cmd = "movq";
	} else {
		move_cmd = "movl";
	}
	if(from_pointer && too_pointer) {
		char *c = get_free_register(size);
		printf("    %s    %s, %s\n", move_cmd, from, c);
		printf("    %s    %s, %s\n", move_cmd, c, too);
		free_register(c);
	} else if (!same) {
		printf("    %s    %s, %s\n", move_cmd, from, too);
	}
}

void comp(char *loc1, char *loc2)
{
	char *reg = NULL;

	/*
		0 == constant (eg. $10)
		1 == register (eg. %esi)
		2 == stack    (eg. -10(%rbp) )
		3 == label    (eg. .L0 )
	*/
	int l1_type, l2_type;
	if(loc1[0] == '$')      l1_type = 0;
	else if(loc1[0] == '%') l1_type = 1;
	else if(loc1[0] == '.') l1_type = 3;
	else					l1_type = 2;

	if(loc2[0] == '$')      l2_type = 0;
	else if(loc2[0] == '%') l2_type = 1;
	else if(loc2[0] == '.') l2_type = 3;
	else					l2_type = 2;

	if(l1_type == 0 && l2_type == 0) {
		//This will fail and needs to be hard coded (as they are both constants)
	} else if(l1_type == 2 && l2_type == 2) {
		// Both in memory so we need to move one to a register
		reg = get_free_register(4);
		move_values(loc1, reg, 4);
		loc1 = reg;
	}
	
	if( (l2_type == 1 && l1_type != 1) || (l2_type == 0 && l1_type != 0) ) {
		char *temp = loc1;
		loc1 = loc2;
		loc2 = temp;
	}
	
	printf("    cmpl    %s, %s\n", loc1, loc2);

	if(reg != NULL) {
		free_register(reg);
	}
}

char* to_value(ptree *tree)
{
	int i1, i2;
	char *c, *c2;
	switch(tree->type) {
		case NODE_INT:
			c = malloc(sizeof(char)*15);
			c[0] = '$';
			sprintf(c+1, "%d", tree->body.a_int);
			return c;
		case NODE_FUNCTION_CALL:;
			int i;
			for(i=0; i<6; i++) {
				free_location(local_vars, get_paramiter_location(i));
			}

			c = malloc(sizeof(char)*20);
			i1 = functions_args_prep;
			if(functions_args_prep >0) {
				// The function paramiters are important so save them in the stack
				printf("    subl    $%d, %%esp\n", i1*8);
				i2 = i1;
				for(;i2>0; i2--) {
					printf("    movl    %s, %d(%%esp)\n", get_paramiter_location(i2-1), i2*8); 
				}
				functions_args_prep = 0;
				prep_function_args(tree->body.a_parent.right, 0);
				printf("    call    %s\n", tree->body.a_parent.left->body.a_string);
				i2 = i1;
				for(;i2>0; i2--) {
					printf("    movl    %d(%%esp), %s\n", i2*8, get_paramiter_location(i2-1));
				}
				functions_args_prep = i2;
				printf("    addl    $%d, %%esp\n", i1*8);
			} else {
				prep_function_args(tree->body.a_parent.right, 0);
				printf("    call    %s\n", tree->body.a_parent.left->body.a_string);
			}

			functions_args_prep = 0;
			sprintf(c, "%%eax");
			return c;
		case NODE_VAR:
			c = get_local_var_location(local_vars, tree->body.a_string);
			return c;
			break;
		case NODE_VAR_POINTER:
			c = get_local_var_location(local_vars, tree->body.a_string);
			printf("    movq    %s, %%rax\n", c);
			printf("    movl    (%%rax), %%eax\n");
			c2 = get_stack_space(4);
			printf("    movl    %%eax, %s\n", c2);
			return c2;
			break;
		case NODE_VAR_TO_POINTER:
			c = get_local_var_location(local_vars, tree->body.a_string);
			printf("    leaq    %s, %%rax\n", c);
			c2 = get_stack_space(8);
			printf("    movq    %%rax, %s\n", c2);
			return c2;
			break;
		case NODE_ADD:
			move_values(to_value(tree->body.a_parent.left), "%r10d", 4);
			printf("    addl    %s, %%r10d\n", to_value(tree->body.a_parent.right));
			c = malloc(sizeof(char)*20);
			sprintf(c, "%%r10d");
			return c;
			break;
		case NODE_SUB:
			move_values(to_value(tree->body.a_parent.left), "%r10d", 4);
			printf("    subl    %s, %%r10d\n", to_value(tree->body.a_parent.right));
			c = malloc(sizeof(char)*20);
			sprintf(c, "%%r10d");
			return c;
			break;
		case NODE_MULT:
			move_values(to_value(tree->body.a_parent.left), "%r10d", 4);
			printf("    imull    %s, %%r10d\n", to_value(tree->body.a_parent.right));
			c = malloc(sizeof(char)*20);
			sprintf(c, "%%r10d");
			return c;
			break;
		case NODE_U_MINUS:
			c = to_value(tree->body.a_parent.left);
			printf("    movl    %s, %%r10d\n", c);
			printf("    negl    %%r10d\n");
			printf("    movl    %%r10d, %s\n", c);
			return c;
			break;
		case NODE_COMP_GT:
			c = get_stack_space(4);
			label_number += 1;
			printf("    movl    $1, %s\n", c);
			comp(to_value(tree->body.a_parent.left), to_value(tree->body.a_parent.right));
			printf("    jg      .L%d\n", label_number-1);
			printf("    movl    $0, %s\n", c);
			printf(".L%d:\n", label_number-1);
			return c;
			break;
		case NODE_COMP_LT:
			c = get_stack_space(4);
			label_number += 1;
			printf("    movl    $1, %s\n", c);
			comp(to_value(tree->body.a_parent.left), to_value(tree->body.a_parent.right));
			printf("    jl      .L%d\n", label_number-1);
			printf("    movl    $0, %s\n", c);
			printf(".L%d:\n", label_number-1);
			return c;
			break;
		case NODE_COMP_EQ:
			c = get_stack_space(4);
			label_number += 1;
			printf("    movl    $1, %s\n", c);
			comp(to_value(tree->body.a_parent.left), to_value(tree->body.a_parent.right));
			printf("    je      .L%d\n", label_number-1);
			printf("    movl    $0, %s\n", c);
			printf(".L%d:\n", label_number-1);
			return c;
			break;
		case NODE_COMP_NEQ:
			c = get_stack_space(4);
			label_number += 1;
			printf("    movl    $1, %s\n", c);
			comp(to_value(tree->body.a_parent.left), to_value(tree->body.a_parent.right));
			printf("    jne     .L%d\n", label_number-1);
			printf("    movl    $0, %s\n", c);
			printf(".L%d:\n", label_number-1);
			return c;
			break;
		case NODE_COMP_GTEQ:
			c = get_stack_space(4);
			label_number += 1;
			printf("    movl    $1, %s\n", c);
			comp(to_value(tree->body.a_parent.left), to_value(tree->body.a_parent.right));
			printf("    jge     .L%d\n", label_number-1);
			printf("    movl    $0, %s\n", c);
			printf(".L%d:\n", label_number-1);
			return c;
			break;
		case NODE_COMP_LTEQ:
			c = get_stack_space(4);
			label_number += 1;
			printf("    movl    $1, %s\n", c);
			comp(to_value(tree->body.a_parent.left), to_value(tree->body.a_parent.right));
			printf("    jle     .L%d\n", label_number-1);
			printf("    movl    $0, %s\n", c);
			printf(".L%d:\n", label_number-1);
			return c;
			break;
		case NODE_STRING_CONST:
			c = malloc(sizeof(char) * 25);
			sprintf(c, "%s", tree->body.a_string);
			return c;
			break;
		default:
			return NULL;
	}
}

char* get_paramiter_location(int n)
{
	return function_arg_locations[n];
}

char* get_paramiter_location_by_size(int n, int size)
{
	switch(size) {
		case 8:
			return function_arg_locations_8[n];
		default:
			return function_arg_locations[n];
	}
}

int prep_function_args(ptree *tree, int argNumber)
{
	if(tree == NULL) return 0;

	//TODO if there are things in the function_args_section (eg. args for the current function) then move them (and their refs...
		// from within local_vars)

	if(argNumber == 0 && tree->type != NODE_FUNCTION_ARG_CHAIN) {
		printf("    movl   %s, %s\n", to_value(tree), get_paramiter_location(0));
		functions_args_prep++;
		return argNumber;
	}

	if(tree->body.a_parent.left->type == NODE_FUNCTION_ARG_CHAIN) {
		argNumber = prep_function_args(tree->body.a_parent.left, argNumber);
	} else {
		char *val = to_value(tree->body.a_parent.left);
		int size = get_var_size_by_location(local_vars, val);
		move_values(val, get_paramiter_location_by_size(argNumber, size), size);
		argNumber++;
		functions_args_prep++;
	}
	if(tree->body.a_parent.right->type == NODE_FUNCTION_ARG_CHAIN) {
		argNumber = prep_function_args(tree->body.a_parent.right, argNumber);
	} else {
		char *val = to_value(tree->body.a_parent.right);
		int size = get_var_size_by_location(local_vars, val);
		move_values(val, get_paramiter_location_by_size(argNumber, size), size);
		argNumber++;
		functions_args_prep++;
	}
	return argNumber;
}

void compile_tree_aux(ptree *tree)
{
	if(tree == NULL) printf("# NULL");

	char *c1;
	ptree *n1;
	int i1;
	bool b1;
	TYPE_DEF type;

	switch(tree->type) {
		case NODE_FUNCTION_DEF:
			//b1 == is it going to return a pointer?
			b1 = tree->body.a_parent.left->type == NODE_FUNCTION_TYPE_POINTER_NAME_PAIR;
			function_return_size = b1 ? 8 : 4;
			params_set = 0;
			printf("%s:\n", tree->body.a_parent.left->body.a_parent.right->body.a_string);
			printf("    pushq   %%rbp\n");         // Save the frame pointer to the stack so we can put it back later
			printf("    movq    %%rsp,  %%rbp\n"); // Change to our own frame pointer

			local_vars = malloc(sizeof(VAR_STORE));
			init_local_store(local_vars);

			compile_tree_aux(tree->body.a_parent.right);
			empty_stack_of_local_vars();
			printf("    popq    %%rbp\n");		// Put back the frame pointer
			printf("    ret\n");
			break;
		case NODE_FUNCTION_PARAM_BODY_PAIR:
			if(tree->body.a_parent.left != 0) compile_tree_aux(tree->body.a_parent.left);
			int i;
			for(i=0; i<6; i++) {
				free_location(local_vars, get_paramiter_location(i));
				free_location(local_vars, function_arg_locations_8[i]);
			}
			compile_tree_aux(tree->body.a_parent.right);
			break;
		case NODE_RETURN:
			c1 = to_value(tree->body.a_parent.left);
			if(function_return_size == 8) {
				printf("    movq    %s, %%rax\n", c1);
			} else {
				printf("    movl    %s,  %%eax\n", c1 );
			}
			break;
		case NODE_MAIN_EXTENDED:
			compile_tree_aux(tree->body.a_parent.left);
			compile_tree_aux(tree->body.a_parent.right);
			break;
		case NODE_FUNCTION_CALL:
			//TODO sort out paramiters in right subtree
			prep_function_args(tree->body.a_parent.right, 0);
			printf("    xor     %%eax, %%eax\n");
			printf("    call    %s\n", tree->body.a_parent.left->body.a_string);
			functions_args_prep = 0;
			break;
		case NODE_PARAMITER_DEF:
		case NODE_PARAMITER_DEF_POINTER:
			type.name = tree->body.a_parent.left->body.a_string;
			type.pointer = false;
			type.size = tree->type==NODE_PARAMITER_DEF_POINTER ? 8 : 4; //TODO
			char *param_location = type.size ==8 ? function_arg_locations_8[params_set] : get_paramiter_location(params_set);
			params_set++;
			add_local_var(local_vars, tree->body.a_parent.right->body.a_string, param_location, type);
			break;
		case NODE_PARAMITER_CHAIN:
			compile_tree_aux(tree->body.a_parent.left);
			compile_tree_aux(tree->body.a_parent.right);
			break;
		case NODE_VAR_ASSIGN:
			set_local_var(local_vars, tree->body.a_parent.left->body.a_string, to_value(tree->body.a_parent.right));
			break;
		case NODE_VAR_ASSIGN_POINTER:
			set_local_var_pointer(local_vars, tree->body.a_parent.left->body.a_string, to_value(tree->body.a_parent.right));
			break;
		case NODE_VAR_DEF:
			type.name = tree->body.a_parent.left->body.a_string;
			type.pointer = false;
			type.size = 4; //TODO
			create_local_var(local_vars, tree->body.a_parent.right->body.a_string, type); //TODO use type to find size
			break;
		case NODE_VAR_DEF_POINTER:
			type.name = tree->body.a_parent.left->body.a_string;
			type.pointer = true;
			type.size = 8; //TODO
			create_local_var(local_vars, tree->body.a_parent.right->body.a_string, type); //TODO use type to find size
			break;
		case NODE_VAR_GLOBAL_DEF:
			c1 = tree->body.a_parent.left->body.a_parent.right->body.a_string; //var name
			n1 = tree->body.a_parent.right;
			printf("\n\n"\
			"    .globl	%s\n"\
			"    .data\n"\
			"    .align 4\n"\
			"    .type	%s, @object\n"\
			"    .size	%s, 4\n"\
			"%s:\n"\
			"    .long	%d\n"\
			"    .text\n\n", c1,c1,c1,c1, n1==NULL?0:n1->body.a_int);
			break;
		case NODE_IF:
			if(tree->body.a_parent.extra == 0) {
				compile_tree_aux(tree->body.a_parent.left);
			} else {
				i1 = label_number;
				label_number += 2;
				c1 = to_value(tree->body.a_parent.extra);
				printf("    cmpl    $0, %s\n", c1);
				printf("    je      .L%d\n", i1);
				compile_tree_aux(tree->body.a_parent.left);
				printf("    jmp     .L%d\n", i1+1);
				printf(".L%d:\n", i1);
				compile_tree_aux(tree->body.a_parent.right);
				printf(".L%d:\n", i1+1);
			}
			break;
	//	case NODE_FUNCTION_ARG_CHAIN:
	//		prep_function_args(tree, 1);
	//		break;
/*		case NODE_INT:
			printf("Int %i\n", root->body.a_int);
			break;
		case NODE_STRING:
			printf("String '%s'\n", root->body.a_string);
			break;
		case NODE_FLOAT:
			printf("Float %f\n", root->body.a_float);
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
			break;*/
		default:
			printf("#Unknown %d\n", tree->type);
			break;
	}
}

void compile_tree(ptree *tree)
{
	create_head(tree);
	compile_tree_aux(tree);
}

