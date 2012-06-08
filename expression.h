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
#include <stack>
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

static llvm::Module *theModule;
static llvm::IRBuilder<> builder(llvm::getGlobalContext());
static llvm::Function *theFunction;
static llvm::Function* func_malloc;
static llvm::TargetData *targetData;

class Expression;
static vector<Expression*> expressions;
class Variable;
static stack<map<string,vector<Variable*> > > variables;

class Type;

class Expression {
public:
	Expression();
	virtual ~Expression();
	virtual ostream & print(ostream& os) const = 0;
	Type *type();
	virtual llvm::Value *codegen() = 0;
	virtual Expression *setExpression(Expression *value);
protected:
	Type *m_type;
};

class Variable {
public:
	Variable();
	Variable(Type *variabletype, llvm::Value *alloc);
	~Variable();
	llvm::Value *alloc();
	Type *variableType();
protected:
	Type *m_variabletype;
	llvm::Value *m_alloc;
};

class NumberExpression : public Expression {
public:
	NumberExpression(int number);
	virtual ~NumberExpression();
	virtual ostream & print(ostream &os) const;
	virtual llvm::Value* codegen();
private:
	int m_number;
};

class BinaryExpression : public Expression {
public:
	BinaryExpression(char op, Expression *a, Expression *b);
	virtual ~BinaryExpression();
	virtual ostream & print(ostream &os) const;
	virtual llvm::Value* codegen();
private:
	char m_op;
	Expression *m_a, *m_b;
};

class VariableExpression : public Expression {
public:
	VariableExpression(const string &name);
	virtual ~VariableExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
	virtual Expression* setExpression(Expression* value);
protected:
	string m_name;
};

class VariableSetExpression : public Expression {
public:
	VariableSetExpression(const string &name, Expression *value);
	virtual ~VariableSetExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
private:
	string m_name;
	Expression *m_value;
};

class VariableDeclarationExpression : public Expression, public Variable {
public:
	VariableDeclarationExpression(Type *variabletype, const string &name, Expression *block);
	virtual ~VariableDeclarationExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
private:
	string m_name;
	Expression *m_block;
};

class WhileExpression : public Expression {
public:
	WhileExpression(Expression *condition, Expression *block);
	virtual ~WhileExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
private:
	Expression *m_condition;
	Expression *m_block;
};

class IfExpression : public Expression {
public:
	IfExpression(Expression *condition, Expression *block, Expression *elseblock = 0);
	virtual ~IfExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
private:
	Expression *m_condition;
	Expression *m_block;
	Expression *m_elseblock;
};

class ArrayExpression : public Expression {
public:
	ArrayExpression(Type *elementtype, Expression *size);
	virtual ~ArrayExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
private:
	Type *m_elementtype;
	Expression *m_size;
};

class ArrayAccessExpression : public Expression {
public:
	ArrayAccessExpression(Expression *array, Expression *index);
	virtual ~ArrayAccessExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
	virtual Expression* setExpression(Expression* value);
private:
	Expression *m_array;
	Expression *m_index;
};

class ArraySetExpression : public Expression {
public:
	ArraySetExpression(Expression *array, Expression *index, Expression *value);
	virtual ~ArraySetExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
private:
	Expression *m_array;
	Expression *m_index;
	Expression *m_value;
};

class Argument {
public:
	Argument(Type *type, const string &name);
	~Argument();
private:
	Type *m_type;
	string m_name;
	friend class FunctionExpression;
};

class ClosureVariable : public VariableExpression {
public:
	ClosureVariable(const string &name, bool reference);
	~ClosureVariable();
	string name();
	bool reference();
	virtual llvm::Value* codegen();
	llvm::Type* realType();
	Variable* createVariable(llvm::Value *in);
private:
	bool m_reference;
	llvm::Type *m_realtype;
};

class FunctionExpression : public Expression {
public:
	FunctionExpression(Type* returntype, const vector<Argument*> &arguments, const vector<ClosureVariable*> &closurelist, Expression* block);
	virtual ~FunctionExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
private:
	Type *m_returntype;
	vector<Argument*> m_arguments;
	vector<ClosureVariable*> m_closurelist;
	Expression *m_block;
};

class CallExpression : public Expression {
public:
	CallExpression(Expression *function, const vector<Expression*> &arguments);
	virtual ~CallExpression();
	virtual ostream& print(ostream& os) const;
	virtual llvm::Value* codegen();
private:
	Expression *m_function;
	vector<Expression*> m_arguments;
};

ostream & operator<<(ostream &os, const Expression &e);

void init();

void handleStatement(Expression *e);

void finalize();

#endif // EXPRESSION_H
