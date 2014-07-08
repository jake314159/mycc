

main: mycc

parse_tree.o: parse_tree.c
	gcc -c parse_tree.c

compiler.o: compiler.c
	gcc -c compiler.c

mycc: lex.yy.c y.tab.c parse_tree.o compiler.o variable_store.o
	gcc lex.yy.c y.tab.c parse_tree.o compiler.o variable_store.o -o mycc -lfl

y.tab.c: parser.y
	yacc -d parser.y

lex.yy.c: lexer.lex y.tab.c
	lex lexer.lex

variable_store.o: variable_store.c
	gcc -c variable_store.c
