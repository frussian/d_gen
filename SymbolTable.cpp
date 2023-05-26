//
// Created by Anton on 26.05.2023.
//

#include "SymbolTable.h"

SymbolTable::SymbolTable(SymbolTable *outer_table): outer_table(outer_table) {}

void SymbolTable::add_symbol(std::string name, std::shared_ptr<Symbol> symbol) {
	table[name] = symbol;
}

std::shared_ptr<Symbol> SymbolTable::find_symbol(const std::string &name) {
	auto it = table.find(name);
	if (it != table.end()) {
		return it->second;
	}
	if (!outer_table) {
		return nullptr;
	}
	return outer_table->find_symbol(name);
}
