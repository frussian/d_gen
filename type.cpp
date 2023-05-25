//
// Created by Anton on 24.05.2023.
//

#include <iterator>

#include "type.h"
#include "utils/assert.h"

Type::Type(std::shared_ptr<std::vector<TypeKind>> types): types(types) {}

TypeKind Type::getCurrentType() {
	if (pos >= types->size()) {
		return TypeKind::INVALID;
	}
	return types->at(pos);
}

Type Type::dropType() {
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
	for (const auto &type: (*types)) {
		switch (type) {
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

	return str;
}
