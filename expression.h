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


#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <ostream>
#include <map>
#include <string>
using namespace std;

#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/PassManager.h"
#include "llvm/Target/TargetData.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Transforms/Scalar.h"
using namespace llvm;

static Module *theModule;
static IRBuilder<> builder(getGlobalContext());
static map<string,AllocaInst*> variables;
static Function *theFunction;

class Expression;
static vector<Expression*> expressions;

class Expression {
public:
	Expression();
	virtual ~Expression();
	virtual ostream & print(ostream& os) const = 0;
	virtual Value *codegen() = 0;
	virtual Expression *setExpression(Expression *value);
};

class NumberExpression : public Expression {
public:
	NumberExpression(int number);
	virtual ~NumberExpression();
	virtual ostream & print(ostream &os) const;
	virtual Value* codegen();
private:
	int m_number;
};

class BinaryExpression : public Expression {
public:
	BinaryExpression(char op, Expression *a, Expression *b);
	virtual ~BinaryExpression();
	virtual ostream & print(ostream &os) const;
	virtual Value* codegen();
private:
	char m_op;
	Expression *m_a, *m_b;
};

class VariableExpression : public Expression {
public:
	VariableExpression(const string &name);
	virtual ~VariableExpression();
	virtual ostream& print(ostream& os) const;
	virtual Value* codegen();
	virtual Expression* setExpression(Expression* value);
private:
	string m_name;
};

class VariableSetExpression : public Expression {
public:
	VariableSetExpression(const string &name, Expression *value);
	virtual ~VariableSetExpression();
	virtual ostream& print(ostream& os) const;
	virtual Value* codegen();
private:
	string m_name;
	Expression *m_value;
};

class WhileExpression : public Expression {
public:
	WhileExpression(Expression *condition, Expression *block);
	virtual ~WhileExpression();
	virtual ostream& print(ostream& os) const;
	virtual Value* codegen();
private:
	Expression *m_condition;
	Expression *m_block;
};

ostream & operator<<(ostream &os, const Expression &e);

void init();

void handleStatement(Expression *e);

void finalize();

#endif // EXPRESSION_H
