//
// Created by Anton on 24.05.2023.
//

#ifndef D_GEN_TYPE_H
#define D_GEN_TYPE_H

#include <vector>
#include <memory>

#include "antlr4-runtime.h"

enum class TypeKind {
	INT,
	STRING,
	CHAR,
	BOOL,
	ARR,
	INVALID
};

class Type {
public:
	std::shared_ptr<std::vector<TypeKind>> types;
	int pos = 0;
	explicit Type(std::shared_ptr<std::vector<TypeKind>> types);
	TypeKind getCurrentType();
	Type dropType();
	static Type create(std::vector<antlr4::Token*> &tokens);
	std::string to_string();
private:
	static TypeKind map_type(const std::string &type);
};


#endif //D_GEN_TYPE_H
