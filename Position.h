//
// Created by Anton on 26.05.2023.
//

#ifndef D_GEN_POSITION_H
#define D_GEN_POSITION_H

class Position {
public:
	int line, col;
	Position(int line, int col): line(line), col(col) {};
	friend std::ostream& operator<<(std::ostream& os, const Position& pos);
};


#endif //D_GEN_POSITION_H
