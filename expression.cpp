/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Fabian Gundlach <fabian.gundlach@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "expression.h"
#include <iostream>

Value *ErrorV(const char *str) { cerr << str << endl; return 0; }

Expression::Expression() {

}

Expression::~Expression() {

}

NumberExpression::NumberExpression(int number) {
	m_number = number;
}

NumberExpression::~NumberExpression() {

}

Value* NumberExpression::codegen() {
	return ConstantInt::get(getGlobalContext(), APInt(32,m_number,true));
}

ostream& NumberExpression::print(ostream& os) const {
	return os << m_number;
}

BinaryExpression::BinaryExpression(char op, Expression* a, Expression* b) {
	m_op = op;
	m_a = a;
	m_b = b;
}

BinaryExpression::~BinaryExpression() {

}

ostream& BinaryExpression::print(ostream& os) const {
	return os << *m_a << m_op << *m_b;
}

Value* BinaryExpression::codegen() {
	Value *va = m_a->codegen();
	Value *vb = m_b->codegen();
	if (va == 0 || vb == 0) return 0;
	switch (m_op) {
	case '+': return Builder.CreateAdd(va, vb, "addtmp");
	case '-': return Builder.CreateSub(va, vb, "subtmp");
	case '*': return Builder.CreateMul(va, vb, "multmp");
	case '/': return Builder.CreateSDiv(va, vb, "divtmp");
	case '%': return Builder.CreateSRem(va, vb, "modtmp");
	default: return ErrorV("invalid binary operator");
	}
}

VariableExpression::VariableExpression(const string& name) {
	m_name = name;
}

VariableExpression::~VariableExpression() {

}

ostream& VariableExpression::print(ostream& os) const {
	return os << m_name;
}

Value* VariableExpression::codegen() {
	if (!Variables.count(m_name))
		return ErrorV("Undefined Variable");
	return Variables[m_name];
}

ostream& operator<<(ostream& os, const Expression& e) {
	os << "(";
	e.print(os);
	return os << ")";
}

void init() {
	Variables["zwei"] = ConstantInt::get(getGlobalContext(), APInt(32,2,true));
}

void handleStatement(Expression* e) {
	cout << *e << endl;
	e->codegen()->dump();
}
