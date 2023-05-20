grammar d_gen;

file        : (name NEWLINE (TAB elements+=action NEWLINE?)* ) EOF;

name        : NAME ':';

action      : DRAW size shape IN color AT position
            | WRITE size STRING IN color AT position
            ;

size        : SMALL | MEDIUM | BIG ;
shape       : CIRCLE | SQUARE;
color       : BLACK | BLUE | BROWN | GREEN | RED | ORANGE | PURPLE | YELLOW | WHITE ;
position    : x=(LEFT | CENTER | RIGHT) ',' y=(TOP | CENTER | BOTTOM) ;

type         : (types += SCALAR_TYPE types += NESTED_TYPE* );

//types
NESTED_TYPE  : '[]' ;
SCALAR_TYPE  : 'int' | 'string' | 'char' ;

STRING       : '"' .*? '"' ;

IDENT        : [a-zA-Z][a-zA-Z0-9\-_]* ;
NUM          : [1-9][0-9]* ;
CHAR         : '\'' (. | '\\n' | '\\t') '\'' ;

NEWLINE      : ('\r'? '\n' | '\r')+ ;
WHITESPACE   : [ \t] -> skip ;
