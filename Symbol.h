//
// Created by Anton on 26.05.2023.
//

#ifndef D_GEN_SYMBOL_H
#define D_GEN_SYMBOL_H

#include "type.h"
#include "Position.h"

class Symbol {
public:
	Position pos;
	Type type;
	std::string name;
	//llvm::Value
	explicit Symbol(Position pos, Type type, std::string name);
};


#endif //D_GEN_SYMBOL_H
