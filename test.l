%{
#include "y.tab.h"
%}
D               [0-9]
number          {D}+(\.{D}+)?
%%
{number}        { return T_NUM; }
[-/+*()\n]      { return yytext[0]; }
.               { return 0; }
%%

int yywrap(void) { 
    return 1;
}
