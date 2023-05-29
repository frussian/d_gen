//
// Created by Anton on 24.05.2023.
//

#include <iterator>
#include <utility>

#include "type.h"
#include "utils/assert.h"

Type::Type(std::shared_ptr<std::vector<TypeKind>> types): types(std::move(types)) {}

TypeKind Type::getCurrentType() const {
	if (pos >= types->size()) {
		return TypeKind::INVALID;
	}
	return types->at(pos);
}

Type Type::dropType() {
	if (*this == TypeKind::STRING) {
		return TypeKind::CHAR;
	}
	auto dropped = *this;
	dropped.pos++;
	return dropped;
}

TypeKind Type::map_type(const std::string &type) {
	if (type == "int") {
		return TypeKind::INT;
	} else if (type == "string") {
		return TypeKind::STRING;
	} else if (type == "char") {
		return TypeKind::CHAR;
	} else if (type == "bool") {
		return TypeKind::BOOL;
	} else if (type == "[]") {
		return TypeKind::ARR;
	}

	ASSERT(false, "invalid type " + type);
}

Type Type::create(std::vector<antlr4::Token *> &tokens) {
	std::vector<TypeKind> types;
	types.reserve(tokens.size());
	for (auto it = tokens.rbegin(); it != tokens.rend(); ++it) {
		types.push_back(map_type((*it)->getText()));
	}

	return Type(std::make_shared<std::vector<TypeKind>>(std::move(types)));
}

std::string Type::to_string() {
	std::string str;
	for (int i = pos; i < types->size(); i++) {
		switch (types->at(i)) {
			case TypeKind::INT: {
				str += "int";
				break;
			}
			case TypeKind::STRING: {
				str += "string";
				break;
			}
			case TypeKind::CHAR: {
				str += "char";
				break;
			}
			case TypeKind::BOOL: {
				str += "bool";
				break;
			}
			case TypeKind::ARR: {
				str += "[]";
				break;
			}
			case TypeKind::INVALID: {
				str += "invalid";
				break;
			}
		}
	}

	if (str.empty()) {
		return "invalid";
	}

	return str;
}

Type::Type(TypeKind type) {
	std::vector<TypeKind> vec = {type};
	types = std::make_shared<std::vector<TypeKind>>(std::move(vec));
}

bool Type::operator==(const Type &rhs) const {
	if (length() != rhs.length()) {
		return false;
	}
	for (int i = 0; i < length(); i++) {
		if (types->at(i + pos) != rhs.types->at(i + rhs.pos)) {
			return false;
		}
	}
	return true;
}

bool Type::operator!=(const Type &rhs) const {
	return !operator==(rhs);
}

bool Type::operator==(const TypeKind &rhs) const {
	if (length() > 1) {
		return false;
	}

	return getCurrentType() == rhs;
}

bool Type::operator!=(const TypeKind &rhs) const {
	return !operator==(rhs);
}

bool Type::type_is_numerical(TypeKind kind) {
	return kind == TypeKind::CHAR || kind == TypeKind::INT;
}

bool Type::is_numerical() const {
	return is_scalar() && type_is_numerical(getCurrentType());
}

bool Type::is_scalar() const {
	return length() <= 1;
}

int Type::length() const {
	return (int)types->size() - pos;
}
