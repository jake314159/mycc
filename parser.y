%{
#include <stdio.h>
#include <stdlib.h>
#include "parse_tree.h"
#include "compiler.h"
%}

// define the "terminal symbol" token types I'm going to use, which
// are in CAPS only because it's a convention:
%token INT FLOAT STRING PTREE RETURN

// yacc fundamentally works by asking lex to get the next token, which it returns as
// an object of type "yystype".  But
// the next token could be of an arbitrary data type, so you can define a C union to
// hold each of the types of tokens that lex could return, and yacc will typedef
// "yystype" as the union instead of its default (int):
%union {
	int ival;
	float fval;
	char *sval;
	struct ptree *tval;
}

// and then you just associate each of the defined token types with one of
// the union fields and we're happy:
%token <tval> INT
%token <tval> FLOAT
%token <tval> STRING
%token <tval> PTREE

%type<tval> functionBody start paramiters mainLine paramitersNotZero expr val argList argListNotZero

%%
// this is the actual grammar that yacc will parse, but for right now it's just
// something silly to echo to the screen what yacc gets from lex.  We'll
// make a real one shortly:
start:
	  start mainLine { $$= make_main_extended($1, $2); }
	| mainLine		{ $$=$1; }
	;

mainLine:
	  STRING STRING '(' paramiters ')' '{' functionBody '}' { $$= make_node_function_def($1, $2, $4, $7); }
;
paramiters:
	  paramitersNotZero { $$= $1; }
	| { $$=NULL; }
;
paramitersNotZero:
	  paramitersNotZero ',' STRING STRING { $$= make_node_paramiter_def($1, $3, $4); }
	| STRING STRING { $$=make_node_paramiter_def(NULL, $1, $2); }
;
functionBody:
	  functionBody expr { $$= make_main_extended($1, $2); }
	| expr { $$= $1; }
;
expr:
	  RETURN val ';' { $$= make_return_node($2); }
	| val ';'		{ $$= $1; }
	| STRING STRING '=' val ';' { $$= make_main_extended(make_var_def($1, $2), make_var_assign($2, $4)); }
	| STRING '=' val ';' { $$= make_var_assign($1, $3); }
;

argList:
	  argListNotZero { $$= $1; }
	|   { $$=NULL; }
;

argListNotZero:
	  argListNotZero ',' val	{ $$= make_function_args($1, $3); }
	| val						{ $$= $1; }
;
val:
	  val '+' val  { $$= make_node_add($1, $3); }
	| val '-' val  { $$= make_node_sub($1, $3); }
	| '-' val      { $$= make_node_unary_minus($2); }
	| start INT    { $$= make_node_body($1, $2); }
	| start FLOAT  { $$= make_node_body($1, $2); }
	| start STRING { $$= make_node_body($1, $2); }
	| INT { $$= $1; }
	| FLOAT { $$= $1; }
	| STRING { $$= make_node_var($1); }
	| STRING '(' argList ')'	{ $$= make_function_call($1, $3); }
;


%%
#include <stdio.h>

// stuff from lex that yacc needs to know about:
extern int yylex();
extern int yyparse();
extern FILE *yyin;

main(int argc, char **argv) {
	/*// open a file handle to a particular file:
	FILE *myfile = fopen("a.snazzle.file", "r");
	// make sure it is valid:
	if (!myfile) {
		cout << "I can't open a.snazzle.file!" << endl;
		return -1;
	}
	// set lex to read from it instead of defaulting to STDIN:
	yyin = myfile;*/
	
	// parse through the input until there is no more:
	ptree *t;
	do {
		yyparse();
	} while (!feof(yyin));

	if(argc>=2) { //TODO this is only tempory
		print_last_tree();
	}
	//printf("\n\n## COMPILED ##\n\n");
	compile_tree(getTreeRoot());

	free_tree(getTreeRoot());
}

void yyerror(char *s) {
	printf("EEK, parse error!  Message: %s\n", s);
	// might as well halt now:
	exit(-1);
}
