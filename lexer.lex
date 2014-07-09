%{
#include <stdio.h>
#include "y.tab.h"  // to get the token types that we return
#include "parse_tree.h"
int line_number = 0;
%}
%%
[ \t] ;
\/\/.*\n ;
return {return RETURN;}
[0-9]+\.[0-9]+ { yylval.tval = make_node_float(atof(yytext)); return FLOAT; }
[0-9]+  { yylval.tval = make_node_int(atoi(yytext)); return INT; }
\"[^\"]*\" {
	// we have to copy because we can't rely on yytext not changing underneath us:
	char *res = malloc(sizeof(char) * (strlen(yytext) + 1));
	strcpy(res, yytext);
	yylval.tval = make_node_string_const(res);
	return STRING_CONST;
}
[a-zA-Z0-9_*-]+ {
	// we have to copy because we can't rely on yytext not changing underneath us:
	char *res = malloc(sizeof(char) * (strlen(yytext) + 1));
	strcpy(res, yytext);
	yylval.tval = make_node_string(res);
	return STRING;
}
[\(\)\{\},;=+-]    { return yytext[0]; }
\n {line_number++;}
. ;
%%
