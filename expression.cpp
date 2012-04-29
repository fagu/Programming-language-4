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

Expression* Expression::setExpression(Expression* value) {
	cerr << "You cannot set this!" << endl;
	return 0;
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
	case '+': return builder.CreateAdd(va, vb, "addtmp");
	case '-': return builder.CreateSub(va, vb, "subtmp");
	case '*': return builder.CreateMul(va, vb, "multmp");
	case '/': return builder.CreateSDiv(va, vb, "divtmp");
	case '%': return builder.CreateSRem(va, vb, "modtmp");
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
	if (!variables.count(m_name))
		return ErrorV("Undefined Variable");
	return builder.CreateLoad(variables[m_name], m_name.c_str());
}

Expression* VariableExpression::setExpression(Expression* value) {
	return new VariableSetExpression(m_name, value);
}

VariableSetExpression::VariableSetExpression(const string& name, Expression* value) {
	m_name = name;
	m_value = value;
}

VariableSetExpression::~VariableSetExpression() {

}

ostream& VariableSetExpression::print(ostream& os) const {
	return os << m_name << "=" << *m_value;
}

Value* VariableSetExpression::codegen() {
	Value *v = m_value->codegen();
	if (!v)
		return 0;
	if (!variables.count(m_name)) {
		AllocaInst *alloca = builder.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0, m_name.c_str());
		variables[m_name] = alloca;
	}
	builder.CreateStore(v, variables[m_name]);
	return builder.CreateLoad(variables[m_name], m_name.c_str());
}

WhileExpression::WhileExpression(Expression* condition, Expression* block) {
	m_condition = condition;
	m_block = block;
}

WhileExpression::~WhileExpression() {

}

ostream& WhileExpression::print(ostream& os) const {
	return os << "while" << *m_condition << "," << *m_block;
}

Value* WhileExpression::codegen() {
	BasicBlock *cmploopBB = BasicBlock::Create(getGlobalContext(), "cmploop", theFunction);
	BasicBlock *loopBB = BasicBlock::Create(getGlobalContext(), "loop", theFunction);
	BasicBlock *afterBB = BasicBlock::Create(getGlobalContext(), "afterloop", theFunction);
	builder.CreateBr(cmploopBB);
	builder.SetInsertPoint(cmploopBB);
	Value *v = m_condition->codegen();
	Value *boo = builder.CreateICmpEQ(v, ConstantInt::get(getGlobalContext(), APInt(32,0,true)));
	builder.CreateCondBr(boo, afterBB, loopBB);
	builder.SetInsertPoint(loopBB);
	m_block->codegen();
	builder.CreateBr(cmploopBB);
	builder.SetInsertPoint(afterBB);
	return ConstantInt::get(getGlobalContext(), APInt(32,0,true));
}

ostream& operator<<(ostream& os, const Expression& e) {
	os << "(";
	e.print(os);
	return os << ")";
}

void init() {
}

void handleStatement(Expression* e) {
	expressions.push_back(e);
}

void finalize() {
	InitializeNativeTarget();
	
	theModule = new Module("my cool jit", getGlobalContext());
	
	ExecutionEngine *ee = EngineBuilder(theModule).create();
	
	FunctionPassManager *fpm = new FunctionPassManager(theModule);

	// Set up the optimizer pipeline.  Start with registering info about how the
	// target lays out data structures.
	fpm->add(new TargetData(*ee->getTargetData()));
	// Provide basic AliasAnalysis support for GVN.
	fpm->add(createBasicAliasAnalysisPass());
	// Promote allocas to registers.
	fpm->add(createPromoteMemoryToRegisterPass());
	// Do simple "peephole" optimizations and bit-twiddling optzns.
	fpm->add(createInstructionCombiningPass());
	// Reassociate expressions.
	fpm->add(createReassociatePass());
	// Eliminate Common SubExpressions.
	fpm->add(createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	fpm->add(createCFGSimplificationPass());

	fpm->doInitialization();
	
	FunctionType *ft = FunctionType::get(Type::getInt32Ty(getGlobalContext()),false);
	theFunction = Function::Create(ft, Function::ExternalLinkage, "", theModule);
	BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", theFunction);
	builder.SetInsertPoint(bb);
	Value *v = 0;
	for (Expression *e : expressions) {
		cerr << *e << endl;
		v = e->codegen();
		if (!v)
			break;
	}
	builder.CreateRet(v);
	verifyFunction(*theFunction);
	theFunction->dump();
	fpm->run(*theFunction);
	theFunction->dump();
	
	void *fptr = ee->getPointerToFunction(theFunction);
	int (*fp)() = (int (*)())(intptr_t)fptr;
	cout << "Ergebnis: " << fp() << endl;
}
