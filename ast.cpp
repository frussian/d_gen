//
// Created by Anton on 23.05.2023.
//
#include <iostream>
#include <utility>

#include "ast.h"
#include "utils/assert.h"

#define OFFSET 4

ASTNode::ASTNode(Position pos, std::vector<ASTNode *> children) :
		pos(pos), children(std::move(children)) {}

void ASTNode::print_spaces(std::ostream &out, int offset) {
	for (int i = 0; i < offset; i++) {
		out << " ";
	}
}

void ASTNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "ASTNode" << std::endl;
	for (const auto &child: children) {
		child->print(out, offset + OFFSET);
	}
}

void ASTNode::visitChildren(ASTTraverser cb, std::any ctx) {
	for (const auto &child: children) {
		if (!child) continue;
		//ctx may change => it's updated in the next
		bool traverse_children = cb(child, ctx);
		if (traverse_children) child->visitChildren(cb, ctx);
	}
}

FunctionNode::FunctionNode(Position pos, ASTNode *pre_cond, Type ret_type, std::string name,
						   std::vector<DefNode *> args, BodyNode *body):
	ASTNode(pos, {pre_cond, body}), pre_cond(pre_cond), ret_type(ret_type), name(std::move(name)),
	args(std::move(args)), body(body) {
	for (const auto arg: this->args) {
		children.push_back(arg);
	}
}

void FunctionNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "FunctionNode " << name << ":" << std::endl;
	body->print(out, offset+OFFSET);
}

BodyNode::BodyNode(Position pos, std::vector<ASTNode *> stmts):
	ASTNode(pos), stmts(std::move(stmts)) {
	for (const auto stmt: this->stmts) {
		children.push_back(stmt);
	}
}

void BodyNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "BodyNode:" << std::endl;
	for (const auto &stmt: stmts) {
		stmt->print(out, offset + OFFSET);
	}
}

IfNode::IfNode(Position pos, PrecondNode *precond, ASTNode *cond,
			   BodyNode *body, BodyNode *else_body):
	ASTNode(pos, {precond, cond, body, else_body}), precond(precond), cond(cond), body(body), else_body(else_body) {
}

void IfNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "IfNode:" << std::endl;
	body->print(out, offset + OFFSET);
	if (else_body) {
		else_body->print(out, offset + OFFSET);
	}
}

ForNode::ForNode(Position pos, PrecondNode *precond, ASTNode *pre_asg, ASTNode *cond,
				 ASTNode *inc_asg, BodyNode *body):
	ASTNode(pos, {precond, pre_asg, cond, inc_asg, body}), precond(precond), pre_asg(pre_asg), cond(cond), inc_asg(inc_asg),
	body(body) {}

void ForNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "ForNode:" << std::endl;
	body->print(out, offset + OFFSET);
}

DefNode::DefNode(Position pos, std::string name, Type type, ASTNode *rhs):
	ASTNode(pos, {rhs}), name(std::move(name)), type(type), rhs(rhs) {}

void DefNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "DefNode: " << std::endl;
	print_spaces(out, offset + OFFSET);
	out << type.to_string() << std::endl;
	print_spaces(out, offset + OFFSET);
	out << name << std::endl;
	if (rhs) {
		rhs->print(out, offset + OFFSET);
	}
}

ContinueNode::ContinueNode(Position pos): ASTNode(pos) {}

void ContinueNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "ContinueNode" << std::endl;
}

BreakNode::BreakNode(Position pos): ASTNode(pos) {}

void BreakNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "BreakNode" << std::endl;
}

ReturnNode::ReturnNode(Position pos, ASTNode *expr): ASTNode(pos, {expr}), expr(expr) {}

void ReturnNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "ReturnNode:" << std::endl;
	expr->print(out, offset + OFFSET);
}

AsgNode::AsgNode(Position pos, ASTNode *lhs, ASTNode *rhs):
		ASTNode(pos, {lhs, rhs}), lhs(lhs), rhs(rhs) {}

void AsgNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "AsgNode:" << std::endl;
	lhs->print(out, offset + OFFSET);
	rhs->print(out, offset + OFFSET);
}

AsgNode *AsgNode::create(Position pos, ASTNode *lhs, const std::string &type, ASTNode *rhs) {
	auto t = map_asg_type(type);
	switch (t) {
		case AsgType::SUM:
			rhs = new BinOpNode(pos, BinOpType::SUM, lhs, rhs);
			break;
		case AsgType::SUB:
			rhs = new BinOpNode(pos, BinOpType::SUB, lhs, rhs);
			break;
		case AsgType::MUL:
			rhs = new BinOpNode(pos, BinOpType::MUL, lhs, rhs);
			break;
		case AsgType::DIV:
			rhs = new BinOpNode(pos, BinOpType::DIV, lhs, rhs);
			break;
		default: break;
	}
	return new AsgNode(pos, lhs, rhs);
}

AsgType AsgNode::map_asg_type(const std::string &type) {
	if (type == "=") {
		return AsgType::ASG;
	} else if (type == "+=") {
		return AsgType::SUM;
	} else if (type == "-=") {
		return AsgType::SUB;
	} else if (type == "*=") {
		return AsgType::MUL;
	} else if (type == "/=") {
		return AsgType::DIV;
	}

	ASSERT(false, "invalid asg type " + type);
}

AsgNode *CremNode::create(Position pos, ASTNode *lhs, const std::string &type) {
	BinOpNode *rhs = nullptr;
	switch (map_crem_type(type)) {
		case CremType::INC:
			rhs = new BinOpNode(pos, BinOpType::SUM, lhs, new NumberNode(pos, 1));
			break;
		case CremType::DEC:
			rhs = new BinOpNode(pos, BinOpType::SUB, lhs, new NumberNode(pos, 1));
			break;
	}
	return new AsgNode(pos, lhs, rhs);
}

CremType CremNode::map_crem_type(const std::string &type) {
	if (type == "++") {
		return CremType::INC;
	} else if (type == "--") {
		return CremType::DEC;
	}
	ASSERT(false, "invalid crem type " + type);
}

CharNode::CharNode(Position pos, char ch): ASTNode(pos), ch(ch) {}

CharNode *CharNode::create(Position pos, antlr4::tree::TerminalNode *token) {
	auto ch = token->getText()[0];
	return new CharNode(pos, ch);
}

StringNode::StringNode(Position pos, std::string str): ASTNode(pos), str(std::move(str)) {}

NumberNode::NumberNode(Position pos, int num): ASTNode(pos), num(num) {}

NumberNode *NumberNode::create(Position pos, antlr4::tree::TerminalNode *token) {
	int num = std::atoi(token->getText().c_str());
	return new NumberNode(pos, num);
}

BoolNode::BoolNode(Position pos, bool val): ASTNode(pos), val(val) {}

BoolNode *BoolNode::create(Position pos, antlr4::tree::TerminalNode *token) {
	switch (token->getText()[0]) {
		case 't': return new BoolNode(pos, true); break;
		case 'f': return new BoolNode(pos, false); break;
	}

	ASSERT(false, "invalid bool token " + token->getText());
}

IdentNode::IdentNode(Position pos, std::string name): ASTNode(pos), name(std::move(name)) {}

BinOpNode::BinOpNode(Position pos, BinOpType op_type, ASTNode *lhs, ASTNode *rhs):
	ASTNode(pos, {lhs, rhs}), lhs(lhs), rhs(rhs), op_type(op_type) {}

BinOpType BinOpNode::map_op_type(const std::string& op_type) {
	if (op_type == "+") {
		return BinOpType::SUM;
	} else if (op_type == "-") {
		return BinOpType::SUB;
	} else if (op_type == "||") {
		return BinOpType::OR;
	} else if (op_type == "<") {
		return BinOpType::LT;
	} else if (op_type == "<=") {
		return BinOpType::LE;
	} else if (op_type == ">") {
		return BinOpType::GT;
	} else if (op_type == ">=") {
		return BinOpType::GE;
	} else if (op_type == "==") {
		return BinOpType::EQ;
	} else if (op_type == "!=") {
		return BinOpType::NEQ;
	} else if (op_type == "*") {
		return BinOpType::MUL;
	} else if (op_type == "/") {
		return BinOpType::DIV;
	} else if (op_type == "&&") {
		return BinOpType::AND;
	}

	ASSERT(false, "invalid bin op type " + op_type);
}

BinOpNode* BinOpNode::create(Position pos, std::string op_type, ASTNode *lhs, ASTNode *rhs) {
	return new BinOpNode(pos, map_op_type(op_type), lhs, rhs);
}

void BinOpNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	out << "BinOpNode:" << std::endl;
	lhs->print(out, offset + OFFSET);
	print_spaces(out, offset + OFFSET);
	std::string op_str;
	switch (op_type) {
		case BinOpType::SUM:
			op_str = "+";
			break;
		case BinOpType::SUB:
			op_str = "-";
			break;
		case BinOpType::OR:
			op_str = "||";
			break;
		case BinOpType::LT:
			op_str = "<";
			break;
		case BinOpType::LE:
			op_str = "<=";
			break;
		case BinOpType::GT:
			op_str = ">";
			break;
		case BinOpType::GE:
			op_str = ">=";
			break;
		case BinOpType::EQ:
			op_str = "==";
			break;
		case BinOpType::NEQ:
			op_str = "!=";
			break;
		case BinOpType::MUL:
			op_str = "*";
			break;
		case BinOpType::DIV:
			op_str = "/";
			break;
		case BinOpType::AND:
			op_str = "&&";
			break;
	}
	out << op_str << std::endl;

	rhs->print(out, offset + OFFSET);
}

ArrLookupNode::ArrLookupNode(Position pos, std::string ident_name, std::vector<ASTNode*> idxs):
		ASTNode(pos), idxs(std::move(idxs)) {
	ident = new IdentNode(pos, std::move(ident_name));
	children.push_back(ident);
	for (const auto &idx: this->idxs) {
		children.push_back(idx);
	}
}

void ArrLookupNode::print(std::ostream &out, int offset) {
	print_spaces(out, offset);
	std::cout << "ArrLookup" << std::endl;
	ident->print(out, offset + OFFSET);
	for (const auto &idx: idxs) {
		idx->print(out, offset + OFFSET);
	}
}

ArrCreateNode::ArrCreateNode(Position pos, Type type, ASTNode *len):
	ASTNode(pos, {len}), type(type), len(len) {}

ArrCreateNode *ArrCreateNode::create(Position pos, Type type, ASTNode *len) {
	auto types = type.types;
	types->insert(types->begin(), TypeKind::ARR);
	return new ArrCreateNode(pos, type, len);
}

PropertyLookupNode::PropertyLookupNode(Position pos, std::string ident_name, std::string property_name):
		ASTNode(pos), property_name(std::move(property_name)) {
	ident = new IdentNode(pos, std::move(ident_name));
	children.push_back(ident);
}

PrecondNode::PrecondNode(Position pos, int prob, ASTNode *expr):
	ASTNode(pos, {expr}), prob(prob), expr(expr) {}
