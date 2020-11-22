%{
#include <stdio.h>
void yyerror(const char* msg);
#define YYSTYPE double
%}

%token T_NUM

%left '+' '-'
%left '*' '/'

%%

S   :   E '\n'          { printf("S->E\n");}
    |   /* empty */     { /* empty */ }
    ;

E   :   E '+' T         { printf("E->E+T\n");}
    |   E '-' T         { printf("E->E-T\n");}
	|   T               { printf("E->T\n");}
    ;
T   :   T '*' F         {  printf("T->T*F\n");}
	|   T '/' F         { printf("T->T/F\n");}
	|   F               { printf("T->F\n");}
	;

F   :   '(' E ')'       { printf("F->(E)\n");}
	|   T_NUM           { printf("F->{num}\n");}
	;
%%

void yyerror(const char* msg){
	puts(msg);
}
int main() {
    return yyparse();
}
