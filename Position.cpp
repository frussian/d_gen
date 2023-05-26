//
// Created by Anton on 26.05.2023.
//

#include <iostream>

#include "Position.h"

std::ostream &operator<<(std::ostream &os, const Position &pos) {
	return os << pos.line << ":" << pos.col;
}
