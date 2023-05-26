//
// Created by Anton on 26.05.2023.
//

#ifndef D_GEN_SYMBOLTABLE_H
#define D_GEN_SYMBOLTABLE_H

#include <unordered_map>

#include "Symbol.h"

class SymbolTable {
private:
	std::unordered_map<std::string, std::shared_ptr<Symbol>> table;
	SymbolTable *outer_table = nullptr;
public:
	explicit SymbolTable() = default;
	explicit SymbolTable(SymbolTable *outer_table);
	void add_symbol(std::string name, std::shared_ptr<Symbol> symbol);
	std::shared_ptr<Symbol> find_symbol(const std::string &name);
};


#endif //D_GEN_SYMBOLTABLE_H
