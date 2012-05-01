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
#include "type.h"
#include <iostream>

llvm::Value *ErrorV(const char *str) { cerr << str << endl; return 0; }

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

llvm::Value* NumberExpression::codegen() {
	return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,m_number,true));
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

llvm::Value* BinaryExpression::codegen() {
	llvm::Value *va = m_a->codegen();
	llvm::Value *vb = m_b->codegen();
	if (va == 0 || vb == 0) return 0;
	switch (m_op) {
	case '+': return builder.CreateAdd(va, vb, "addtmp");
	case '-': return builder.CreateSub(va, vb, "subtmp");
	case '*': return builder.CreateMul(va, vb, "multmp");
	case '/': return builder.CreateSDiv(va, vb, "divtmp");
	case '%': return builder.CreateSRem(va, vb, "modtmp");
	case ';': return vb;
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

llvm::Value* VariableExpression::codegen() {
	if (variables[m_name].empty())
		return ErrorV("Undefined Variable");
	return builder.CreateLoad(variables[m_name].back(), m_name.c_str());
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

llvm::Value* VariableSetExpression::codegen() {
	llvm::Value *v = m_value->codegen();
	if (!v)
		return 0;
	if (variables[m_name].empty())
		return 0;
	builder.CreateStore(v, variables[m_name].back());
	return v;
}

VariableDeclarationExpression::VariableDeclarationExpression(Type* type, const string& name, Expression *block) {
	m_type = type;
	m_name = name;
	m_block = block;
}

VariableDeclarationExpression::~VariableDeclarationExpression() {

}

ostream& VariableDeclarationExpression::print(ostream& os) const {
	return os << *m_type << "->" << m_name << " in " << *m_block;
}

llvm::Value* VariableDeclarationExpression::codegen() {
	llvm::AllocaInst *alloca = builder.CreateAlloca(m_type->codegen(), 0, m_name.c_str());
	variables[m_name].push_back(alloca);
	llvm::Value *returnv = m_block->codegen();
	variables[m_name].pop_back();
	return returnv;
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

llvm::Value* WhileExpression::codegen() {
	llvm::BasicBlock *cmploopBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "cmploop", theFunction);
	llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "loop", theFunction);
	llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterloop", theFunction);
	builder.CreateBr(cmploopBB);
	builder.SetInsertPoint(cmploopBB);
	llvm::Value *v = m_condition->codegen();
	llvm::Value *boo = builder.CreateICmpEQ(v, llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)));
	builder.CreateCondBr(boo, afterBB, loopBB);
	builder.SetInsertPoint(loopBB);
	m_block->codegen();
	builder.CreateBr(cmploopBB);
	builder.SetInsertPoint(afterBB);
	return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true));
}

IfExpression::IfExpression(Expression* condition, Expression* block, Expression* elseblock) {
	m_condition = condition;
	m_block = block;
	m_elseblock = elseblock;
}

IfExpression::~IfExpression() {

}

ostream& IfExpression::print(ostream& os) const {
	os << "if" << *m_condition << "," << *m_block;
	if (m_elseblock)
		os << "," << *m_elseblock;
	return os;
}

llvm::Value* IfExpression::codegen() {
	bool ok = true;
	llvm::AllocaInst *result = builder.CreateAlloca(llvm::Type::getInt32Ty(llvm::getGlobalContext()), 0, "ifresult");
	llvm::BasicBlock *ifblockBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "ifblock", theFunction);
	llvm::BasicBlock *elseblockBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "elseblock", theFunction);
	llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterif", theFunction);
	llvm::Value *v = m_condition->codegen();
	llvm::Value *boo = builder.CreateICmpEQ(v, llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)));
	builder.CreateCondBr(boo, elseblockBB, ifblockBB);
	builder.SetInsertPoint(ifblockBB);
	llvm::Value *vs = m_block->codegen();
	if (vs)
		builder.CreateStore(vs, result);
	else
		ok = false;
	builder.CreateBr(afterBB);
	builder.SetInsertPoint(elseblockBB);
	llvm::Value *vse = 0;
	if (m_elseblock) {
		vse = m_elseblock->codegen();
		if (vse)
			builder.CreateStore(vse, result);
		else
			ok = false;
	} else
		builder.CreateStore(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)), result);
	builder.CreateBr(afterBB);
	builder.SetInsertPoint(afterBB);
	if (!ok)
		return 0;
	return builder.CreateLoad(result);
}

ArrayExpression::ArrayExpression(Type* elementtype, Expression* size) {
	m_elementtype = elementtype;
	m_size = size;
}

ArrayExpression::~ArrayExpression() {

}

ostream& ArrayExpression::print(ostream& os) const {
	return os << "new" << *m_elementtype << "[" << *m_size << "]";
}

llvm::Value* ArrayExpression::codegen() {
	llvm::Type *t = m_elementtype->codegen();
	llvm::Type *arrayt = llvm::PointerType::get(t,0);
	llvm::Value *sizev = m_size->codegen();
	llvm::Value *size64v = builder.CreateSExt(sizev, llvm::Type::getInt64Ty(llvm::getGlobalContext()));
	uint64_t bytesperelement = targetData->getTypeAllocSize(t);
	llvm::Value *bytes64v = builder.CreateMul(size64v, llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()),bytesperelement));
	llvm::Value *mallocv = builder.CreateCall(func_malloc, bytes64v, "malloc");
	llvm::Value *pointerv = builder.CreateBitCast(mallocv, arrayt);
	return pointerv;
}

ArrayAccessExpression::ArrayAccessExpression(Expression* array, Expression* index) {
	m_array = array;
	m_index = index;
}

ArrayAccessExpression::~ArrayAccessExpression() {

}

ostream& ArrayAccessExpression::print(ostream& os) const {
	return os << *m_array << "[" << *m_index << "]";
}

llvm::Value* ArrayAccessExpression::codegen() {
	llvm::Value *arrayv = m_array->codegen();
	llvm::Value *indexv = m_index->codegen();
	llvm::Value *pointerv = builder.CreateGEP(arrayv, indexv, "arrayelementpointer");
	return builder.CreateLoad(pointerv, "arrayelement");
}

Expression* ArrayAccessExpression::setExpression(Expression* value) {
	return new ArraySetExpression(m_array, m_index, value);
}

ArraySetExpression::ArraySetExpression(Expression* array, Expression* index, Expression* value) {
	m_array = array;
	m_index = index;
	m_value = value;
}

ArraySetExpression::~ArraySetExpression() {

}

ostream& ArraySetExpression::print(ostream& os) const {
	return os << *m_array << "[" << *m_index << "]=" << *m_value;
}

llvm::Value* ArraySetExpression::codegen() {
	llvm::Value *arrayv = m_array->codegen();
	llvm::Value *indexv = m_index->codegen();
	llvm::Value *pointerv = builder.CreateGEP(arrayv, indexv, "arrayelementpointer");
	llvm::Value *valuev = m_value->codegen();
	builder.CreateStore(valuev, pointerv);
	return valuev;
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
	llvm::InitializeNativeTarget();
	
	theModule = new llvm::Module("my cool jit", llvm::getGlobalContext());
	
	llvm::ExecutionEngine *ee = llvm::EngineBuilder(theModule).create();
	
	targetData = new llvm::TargetData(*ee->getTargetData());
	
	llvm::FunctionPassManager *fpm = new llvm::FunctionPassManager(theModule);

	// Set up the optimizer pipeline.  Start with registering info about how the
	// target lays out data structures.
	fpm->add(targetData);
	// Provide basic AliasAnalysis support for GVN.
	fpm->add(llvm::createBasicAliasAnalysisPass());
	// Promote allocas to registers.
	fpm->add(llvm::createPromoteMemoryToRegisterPass());
	// Do simple "peephole" optimizations and bit-twiddling optzns.
	fpm->add(llvm::createInstructionCombiningPass());
	// Reassociate expressions.
	fpm->add(llvm::createReassociatePass());
	// Eliminate Common SubExpressions.
	fpm->add(llvm::createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	fpm->add(llvm::createCFGSimplificationPass());

	fpm->doInitialization();
	
	llvm::PointerType* voidptrt = llvm::PointerType::get(llvm::IntegerType::get(llvm::getGlobalContext(), 8), 0);
	vector<llvm::Type*> mallocargs;
	mallocargs.push_back(llvm::IntegerType::get(theModule->getContext(), 64));
	llvm::FunctionType* malloctype = llvm::FunctionType::get(voidptrt, mallocargs, false);
	func_malloc = llvm::Function::Create(malloctype, llvm::GlobalValue::ExternalLinkage, "malloc", theModule);
	func_malloc->setCallingConv(llvm::CallingConv::C);
	
	llvm::FunctionType *ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()),false);
	theFunction = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "", theModule);
	llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", theFunction);
	builder.SetInsertPoint(bb);
	llvm::Value *v = 0;
	for (Expression *e : expressions) {
		cerr << *e << endl;
		v = e->codegen();
		if (!v)
			break;
	}
	builder.CreateRet(v);
	theModule->dump();
	verifyFunction(*theFunction);
	fpm->run(*theFunction);
	theModule->dump();
	
	void *fptr = ee->getPointerToFunction(theFunction);
	int (*fp)() = (int (*)())(intptr_t)fptr;
	cout << "Ergebnis: " << fp() << endl;
}
