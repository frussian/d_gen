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
	Type(TypeKind type);
	TypeKind getCurrentType() const;
	Type dropType() const;
	static Type create(std::vector<antlr4::Token*> &tokens);
	std::string to_string();
	bool operator==(const Type& rhs) const;
	bool operator!=(const Type& rhs) const;
	bool operator==(const TypeKind& rhs) const;
	bool operator!=(const TypeKind& rhs) const;
	bool is_numerical() const;
	bool is_scalar() const;
	bool is_string() const;
	bool is_convertable_to(Type &other) const;
	int length() const;
	static bool type_is_numerical(TypeKind kind);
private:
	static TypeKind map_type(const std::string &type);
};


#endif //D_GEN_TYPE_H
