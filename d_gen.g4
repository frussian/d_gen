grammar d_gen;

program      : function NEWLINE* EOF ;
function     : (precondition NEWLINE)? ret_type = type f_name = IDENT
	'(' arg_types += type args += IDENT (',' arg_types += type args += IDENT)* ')' body;

body         : '{' NEWLINE (stmts += statement NEWLINE)* '}';
statement    : assignment | return | break | continue |
               define | for | while | if ;

assignment   : simple_asg | crem_asg ;
simple_asg   : (f_ident | array_lookup) (ASSG | ASSG_TYPE) logic_expr ;
crem_asg     : (f_ident | array_lookup) CREM_TYPE ;
return       : 'return' logic_expr ;
break        : 'break' ;
continue     : 'continue' ;
define       : type IDENT (ASSG logic_expr)? ;
for          : (precondition NEWLINE)? 'for' pre_asg = assignment? ';' logic_expr? ';' inc_asg = assignment? body ;
while        : (precondition NEWLINE)? 'while' logic_expr body ;
if           : (precondition NEWLINE)? 'if' logic_expr body ('else' body)? ;

precondition : '[' ('prob' ASSG NUM ';')? logic_expr? ']' ;

//condition    : tl ( '||' tl )* ;
//tl           : fl ( '&&' fl)* ;
//fl           : '(' condition ')' | expr CMP_OP expr ;

logic_expr       : operands += logic_t ( ops += '||' operands += logic_t)* ;
logic_t          : operands += logic_f ( ops += '&&' operands += logic_f)* ;
logic_f          : logic_inner_expr | cmp_op | expr ;
logic_inner_expr : '(' logic_expr ')' ;
cmp_op           : expr CMP_OP expr ;

expr         : operands += t ( (ops += ('+' | '-')) operands += t )* ;
t            : operands += f ( ops += ('*' | '/') operands += f)* ;
f            : inner_expr | const | f_ident | array_lookup |
               array_create | property_lookup | unary_minus ;

inner_expr      : '(' expr ')' ;
const           : CHAR | STRING | NUM | BOOL;
f_ident         : IDENT ;
array_lookup    : IDENT '[' expr ']' ;
array_create    : type '[' expr ']' ;
property_lookup : IDENT '.' IDENT ;
unary_minus     : '-' f ;

type         : (types += SCALAR_TYPE types += NESTED_TYPE* );

//types
NESTED_TYPE  : '[]' ;
SCALAR_TYPE  : 'int' | 'string' | 'char' | 'bool' ;

ASSG         : '=' ;

ASSG_TYPE    : '+=' | '-=' | '*=' | '/=' ;
CREM_TYPE    : '++' | '--' ;
CMP_OP       : '<' | '<=' | '>' | '>=' | '==' | '!=' ;

IDENT        : [a-zA-Z][a-zA-Z0-9_]* ;
CHAR         : '\'' (. | '\\n' | '\\t') '\'' ;
STRING       : '"' .*? '"' ;
NUM          : [1-9][0-9]* | [0-9] ;
BOOL         : 'true' | 'false' ;

NEWLINE      : ('\r'? '\n' | '\r')+ ;
WHITESPACE   : [ \t] -> skip ;
