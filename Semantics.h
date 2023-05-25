//
// Created by Anton on 25.05.2023.
//

#ifndef D_GEN_SEMANTICS_H
#define D_GEN_SEMANTICS_H

#include "ast.h"

class Semantics {
public:
	static void eliminate_unreachable_code(FunctionNode *func);

private:
	static void eliminate_unreachable_code_visit_body(BodyNode *body);
};


#endif //D_GEN_SEMANTICS_H
