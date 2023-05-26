//
// Created by Anton on 26.05.2023.
//

#include "Symbol.h"

Symbol::Symbol(Position pos, Type type, std::string name):
	pos(pos), type(type), name(std::move(name)) {}
