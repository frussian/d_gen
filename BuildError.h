//
// Created by Anton on 25.05.2023.
//

#ifndef D_GEN_BUILDERROR_H
#define D_GEN_BUILDERROR_H


#include <exception>

#include "ast.h"

struct Err {
	Position pos;
	std::string msg;
};

class BuildError: std::exception {
public:
	std::vector<Err> errors;
	explicit BuildError(std::vector<Err> errors);
};


#endif //D_GEN_BUILDERROR_H
