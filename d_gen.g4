grammar d_gen;

program      : function NEWLINE* EOF ;
function     : (precondition NEWLINE)? type IDENT '(' type IDENT (',' type IDENT)* ')' body;
body         : '{' NEWLINE (stmts += statement NEWLINE)* '}';
statement    : assignment | return | break | continue |
               define | for | while | if ;

assignment   : IDENT ('[' expr ']')? ASSG_TYPE expr | IDENT CREM_TYPE ;
return       : 'return' expr ;
break        : 'break' ;
continue     : 'continue' ;
define       : type IDENT | type assignment ;
for          : (precondition NEWLINE)? 'for' assignment? ';' expr? ';' assignment? body ;
while        : (precondition NEWLINE)? 'while' expr body ;
if           : (precondition NEWLINE)? 'if' expr body ('else' body)? ;

precondition : '[' expr ']' ;

//condition    : tl ( '||' tl )* ;
//tl           : fl ( '&&' fl)* ;
//fl           : '(' condition ')' | expr CMP_OP expr ;

expr         : t ( ('+' | '-' | '||' | CMP_OP) t )* ;
t            : factors += f ( ops += ('*' | '/' | '&&') factors += f)* ;
f            : '(' expr ')' | CONST | IDENT | IDENT '[' expr ']' |
               type '[' expr ']' /*array create*/ | IDENT '.' IDENT /*property*/ ;

type         : (types += SCALAR_TYPE types += NESTED_TYPE* );

//types
NESTED_TYPE  : '[]' ;
SCALAR_TYPE  : 'int' | 'string' | 'char' | 'bool' ;


ASSG_TYPE    : '=' | '+=' | '-=' | '*=' | '/=' ;
CREM_TYPE    : '++' | '--' ;
CMP_OP       : '<' | '<=' | '>' | '>=' | '==' | '!=' ;

CONST        : CHAR | STRING | NUM | BOOL ;

IDENT        : [a-zA-Z][a-zA-Z0-9\-_]* ;
CHAR         : '\'' (. | '\\n' | '\\t') '\'' ;
STRING       : '"' .*? '"' ;
NUM          : '-'?[1-9][0-9]* | '-'?[0-9] ;
BOOL         : 'true' | 'false' ;

NEWLINE      : ('\r'? '\n' | '\r')+ ;
WHITESPACE   : [ \t] -> skip ;
