//
// Created by Anton on 23.05.2023.
//

#ifndef D_GEN_AST_H
#define D_GEN_AST_H

#include "d_genParser.h"

#include "type.h"

class Position {
public:
	int line, col;
	Position(int line, int col): line(line), col(col) {};
};

class ASTNode {
public:
	Position pos;
	explicit ASTNode(Position pos): pos(pos) {};
	virtual ~ASTNode() = default;
	void print_spaces(std::ostream &out, int offset);

	virtual void print(std::ostream &out, int offset);
//	virtual llvm::Value *codegen(CodeGenContext *ctx) {return nullptr;};
};

class DefNode;
class BodyNode;

class FunctionNode: public ASTNode {
public:
	ASTNode *pre_cond;
	Type ret_type;
	std::string name;
	std::vector<DefNode*> args;
	BodyNode *body;
	explicit FunctionNode(Position pos, ASTNode *pre_cond, Type ret_type, std::string name,
						  std::vector<DefNode*> args, BodyNode *body);

	void print(std::ostream &out, int offset) override;
};

class BodyNode: public ASTNode {
public:
	std::vector<ASTNode*> stmts;
	explicit BodyNode(Position pos, std::vector<ASTNode*> stmts);

	void print(std::ostream &out, int offset);
};

class IfNode: public ASTNode {
public:
	ASTNode *precond;
	ASTNode *cond;
	BodyNode *body, *else_body;
	explicit IfNode(Position pos, ASTNode *precond, ASTNode *cond, BodyNode *body, BodyNode *else_body);

	void print(std::ostream &out, int offset);
};

class ForNode: public ASTNode {
public:
	ASTNode *precond;
	ASTNode *pre_asg;
	ASTNode *cond;
	ASTNode *inc_asg;
	BodyNode *body;
	explicit ForNode(Position pos, ASTNode *precond, ASTNode *pre_asg, ASTNode *cond,
					 ASTNode *inc_asg, BodyNode *body);

	void print(std::ostream &out, int offset);
};

class DefNode: public ASTNode {
public:
	//TODO: make IdentNode*
	std::string name;
	Type type;
	ASTNode *rhs;
	explicit DefNode(Position pos, std::string name, Type type, ASTNode *rhs);

	void print(std::ostream &out, int offset);
};

class ContinueNode: public ASTNode {
public:
	//TODO: make ForNode *
	explicit ContinueNode(Position pos);

	void print(std::ostream &out, int offset);
};

class BreakNode: public ASTNode {
public:
	//TODO: make ForNode *
	explicit BreakNode(Position pos);

	void print(std::ostream &out, int offset);
};

class ReturnNode: public ASTNode {
public:
	ASTNode *expr;
	explicit ReturnNode(Position pos, ASTNode *expr);

	void print(std::ostream &out, int offset);
};

enum class AsgType {
	ASG,
	SUM,
	SUB,
	MUL,
	DIV
};

class AsgNode: public ASTNode {
public:
	ASTNode *lhs;
	AsgType type;
	ASTNode *rhs;
	explicit AsgNode(Position pos, ASTNode *lhs, AsgType type, ASTNode *rhs);
	static AsgNode *create(Position pos, ASTNode *lhs, const std::string &type, ASTNode *rhs);
private:
	static AsgType map_asg_type(const std::string &type);

	void print(std::ostream &out, int offset);
};

enum class CremType {
	INC,
	DEC
};

class CremNode: public ASTNode {
public:
	ASTNode *lhs;
	CremType type;
	explicit CremNode(Position pos, ASTNode *lhs, CremType type);
	static CremNode *create(Position pos, ASTNode *lhs, const std::string &type);
private:
	static CremType map_crem_type(const std::string &type);
};

class CharNode: public ASTNode {
public:
	char ch;
	explicit CharNode(Position pos, char ch);
	static CharNode *create(Position pos, antlr4::tree::TerminalNode *token);
};

class StringNode: public ASTNode {
public:
	std::string str;
	explicit StringNode(Position pos, std::string str);
};

class NumberNode: public ASTNode {
public:
	int num;
	explicit NumberNode(Position pos, int num);
	static NumberNode *create(Position pos, antlr4::tree::TerminalNode *token);
};

class BoolNode: public ASTNode {
public:
	bool val;
	explicit BoolNode(Position pos, bool val);
	static BoolNode *create(Position pos, antlr4::tree::TerminalNode *token);
};

class IdentNode: public ASTNode {
public:
	std::string name;
	//type
	explicit IdentNode(Position pos, std::string name);
};

enum class BinOpType {
	//low priority
	SUM,
	SUB,
	OR,
	LT,
	LE,
	GT,
	GE,
	EQ,
	NEQ,

	//first priority
	MUL,
	DIV,
	AND
};

class BinOpNode: public ASTNode {
public:
	ASTNode *lhs, *rhs;
	BinOpType op_type;
	explicit BinOpNode(Position pos, BinOpType op_type, ASTNode *lhs, ASTNode *rhs);
	static BinOpNode *create(Position pos, std::string op_type, ASTNode *lhs, ASTNode *rhs);
private:
	static BinOpType map_op_type(const std::string& op_type);
};

class ArrLookupNode: public ASTNode {
public:
	IdentNode *ident;
	ASTNode *index;
	explicit ArrLookupNode(Position pos, std::string ident_name, ASTNode *index);
};

class ArrCreateNode: public ASTNode {
public:
	Type type;
	ASTNode *len;
	explicit ArrCreateNode(Position pos, Type type, ASTNode *len);
	static ArrCreateNode *create(Position pos, Type type, ASTNode *len);
};

class PropertyLookupNode: public ASTNode {
public:
	IdentNode *ident;
	std::string property_name;
	explicit PropertyLookupNode(Position pos, std::string ident_name, std::string property_name);
};


#endif //D_GEN_AST_H