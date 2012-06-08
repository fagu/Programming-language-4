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
	m_type = 0;
}

Expression::~Expression() {

}

Expression* Expression::setExpression(Expression* value) {
	cerr << "You cannot set this!" << endl;
	return 0;
}

Type* Expression::type() {
	return m_type;
}

Variable::Variable() {

}

Variable::Variable(Type* variabletype, llvm::Value* alloc) {
	m_variabletype = variabletype;
	m_alloc = alloc;
}

Variable::~Variable() {

}

llvm::Value* Variable::alloc() {
	return m_alloc;
}

Type* Variable::variableType() {
	return m_variabletype;
}

NumberExpression::NumberExpression(int number) {
	m_number = number;
}

NumberExpression::~NumberExpression() {

}

ostream& NumberExpression::print(ostream& os) const {
	return os << m_number;
}

llvm::Value* NumberExpression::codegen() {
	m_type = new IntegerType;
	return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,m_number,true));
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
	m_type = new IntegerType;
	llvm::Value *va = m_a->codegen();
	llvm::Value *vb = m_b->codegen();
	if (va == 0 || vb == 0) return 0;
	if (m_op == ';') {
		m_type = m_b->type();
		return vb;
	}
	if (!(*m_a->type() == IntegerType()) || 
	    !(*m_b->type() == IntegerType()))
		return ErrorV("This is not an integer!");
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

llvm::Value* VariableExpression::codegen() {
	if (variables.top()[m_name].empty())
		return ErrorV("Undefined Variable");
	m_type = variables.top()[m_name].back()->variableType();
	return builder.CreateLoad(variables.top()[m_name].back()->alloc(), m_name.c_str());
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
	if (variables.top()[m_name].empty())
		return 0;
	Variable *var = variables.top()[m_name].back();
	if (!(*var->variableType() == *m_value->type()))
		return ErrorV("Types not matching!");
	m_type = var->variableType();
	builder.CreateStore(v, var->alloc());
	return v;
}

VariableDeclarationExpression::VariableDeclarationExpression(Type* variabletype, const string& name, Expression* block) {
	m_variabletype = variabletype;
	m_name = name;
	m_block = block;
}

VariableDeclarationExpression::~VariableDeclarationExpression() {

}

ostream& VariableDeclarationExpression::print(ostream& os) const {
	return os << *m_variabletype << "->" << m_name << " in " << *m_block;
}

llvm::Value* VariableDeclarationExpression::codegen() {
	m_alloc = builder.CreateAlloca(m_variabletype->codegen(), 0, m_name.c_str());
	variables.top()[m_name].push_back(this);
	llvm::Value *returnv = m_block->codegen();
	variables.top()[m_name].pop_back();
	m_type = m_block->type();
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
	m_type = new IntegerType;
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
	if (!m_elseblock || !(*m_block->type() == *m_elseblock->type()))
		m_type = new IntegerType;
	else
		m_type = m_block->type();
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
	m_type = new ArrayType(m_elementtype);
	llvm::Type *t = m_elementtype->codegen();
	llvm::Type *arrayt = m_type->codegen();
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
	ArrayType *at = dynamic_cast<ArrayType*>(m_array->type());
	if (!at)
		return ErrorV("This is not an array!");
	m_type = at->elementType();
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
	ArrayType *at = dynamic_cast<ArrayType*>(m_array->type());
	if (!at)
		return ErrorV("This is not an array!");
	if (!(*at->elementType() == *m_value->type()))
		return ErrorV("Types not matching!");
	m_type = at->elementType();
	return valuev;
}

Argument::Argument(Type* type, const string& name) {
	m_type = type;
	m_name = name;
}

Argument::~Argument() {

}

ClosureVariable::ClosureVariable(const string& name, bool reference) : VariableExpression(name) {
	m_reference = reference;
}

ClosureVariable::~ClosureVariable() {

}

string ClosureVariable::name() {
	return m_name;
}

bool ClosureVariable::reference() {
	return m_reference;
}

llvm::Value* ClosureVariable::codegen() {
	if (!m_reference) {
		llvm::Value *v = VariableExpression::codegen();
		m_realtype = m_type->codegen();
		return v;
	}
	if (variables.top()[m_name].empty())
		return ErrorV("Undefined Variable");
	m_type = variables.top()[m_name].back()->variableType();
	m_realtype = m_type->codegen()->getPointerTo();
	return variables.top()[m_name].back()->alloc();
}

llvm::Type* ClosureVariable::realType() {
	return m_realtype;
}

Variable* ClosureVariable::createVariable(llvm::Value* in) {
	if (!m_reference) {
		llvm::AllocaInst *alloc = builder.CreateAlloca(m_realtype, 0, m_name.c_str());
		builder.CreateStore(in, alloc);
		return new Variable(m_type, alloc);
	} else {
		return new Variable(m_type, in);
	}
}

FunctionExpression::FunctionExpression(Type* returntype, const vector<Argument*> &arguments, const vector<ClosureVariable*> &closurelist, Expression* block) {
	m_returntype = returntype;
	m_arguments = arguments;
	m_closurelist = closurelist;
	m_block = block;
}

FunctionExpression::~FunctionExpression() {

}

ostream& FunctionExpression::print(ostream& os) const {
	os << *m_returntype << ":" << "(";
	for (int i = 0; i < (int)m_arguments.size(); i++) {
		if (i)
			os << ",";
		os << *m_arguments[i]->m_type << m_arguments[i]->m_name;
	}
	os << ")[";
	for (int i = 0; i < (int)m_closurelist.size(); i++) {
		if (i)
			os << ",";
		os << m_closurelist[i]->name();
		if (m_closurelist[i]->reference())
			os << "&";
	}
	return os << "]" << "(" << *m_block << ")";
}

llvm::Value* FunctionExpression::codegen() {
	vector<Type*> argTypes;
	for (Argument *a : m_arguments)
		argTypes.push_back(a->m_type);
	FunctionType *ft = new FunctionType(m_returntype, argTypes);
	m_type = ft;
	llvm::Value *clpv;
	llvm::StructType *closuretype;
	if (!m_closurelist.empty()) {
		vector<llvm::Value*> closurevalues;
		vector<llvm::Type*> closuretypes;
		for (ClosureVariable *a : m_closurelist) {
			closurevalues.push_back(a->codegen());
			closuretypes.push_back(a->realType());
		}
		closuretype = llvm::StructType::get(llvm::getGlobalContext(), closuretypes);
		llvm::Value *sv = builder.CreateAlloca(closuretype);
		int cn = 0;
		for (llvm::Value *a : closurevalues) {
			vector<llvm::Value*> ids;
			ids.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)));
			ids.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,cn,true)));
			builder.CreateStore(a, builder.CreateGEP(sv, ids));
			cn++;
		}
		clpv = builder.CreatePtrToInt(sv, llvm::IntegerType::get(llvm::getGlobalContext(), 64));
	} else {
		clpv = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64,0,true));
	}
	llvm::FunctionType *ftv = ft->functionType();
	llvm::BasicBlock *blockbef = builder.GetInsertBlock();
	llvm::BasicBlock::iterator insertpoint = builder.GetInsertPoint();
	llvm::Function *oldFunction = theFunction;
	llvm::Function *f = llvm::Function::Create(ftv, llvm::Function::ExternalLinkage, "inline", theModule);
	llvm::Type *st = ft->structType();
	llvm::Value *fsv = builder.CreateAlloca(st);
	vector<llvm::Value*> ids;
	ids.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)));
	ids.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)));
	if (!m_closurelist.empty())
		builder.CreateStore(clpv, builder.CreateGEP(fsv, ids));
	ids.pop_back();
	ids.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,1,true)));
	builder.CreateStore(f, builder.CreateGEP(fsv, ids));
	theFunction = f;
	variables.push(map<string,vector<Variable*> >());
	unsigned Idx = 0;
	llvm::Function::arg_iterator AI = f->arg_begin();
	AI->setName("closure");
	AI++;
	for (; Idx != m_arguments.size(); ++AI, ++Idx) {
		AI->setName(m_arguments[Idx]->m_name);
	}
	llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", f);
	builder.SetInsertPoint(bb);
	AI = f->arg_begin();
	if (!m_closurelist.empty()) {
		llvm::Value *cp = builder.CreateIntToPtr(AI, llvm::PointerType::get(closuretype, 0));
		int cn = 0;
		for (ClosureVariable *a : m_closurelist) {
			vector<llvm::Value*> idsi;
			idsi.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)));
			idsi.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,cn,true)));
			Variable *v = a->createVariable(builder.CreateLoad(builder.CreateGEP(cp, idsi)));
			variables.top()[a->name()].push_back(v);
			cn++;
		}
	}
	AI++;
	for (Argument *a : m_arguments) {
		llvm::AllocaInst *alloc = builder.CreateAlloca(a->m_type->codegen(), 0, a->m_name.c_str());
		builder.CreateStore(AI, alloc);
		Variable *v = new Variable(a->m_type, alloc);
		variables.top()[a->m_name].push_back(v);
		AI++;
	}
	llvm::Value *v = m_block->codegen();
	builder.CreateRet(v);
	builder.SetInsertPoint(blockbef, insertpoint);
	theFunction = oldFunction;
	variables.pop();
	return fsv;
}

CallExpression::CallExpression(Expression* function, const std::vector< Expression* >& arguments) {
	m_function = function;
	m_arguments = arguments;
}

CallExpression::~CallExpression() {

}

ostream& CallExpression::print(ostream& os) const {
	os << *m_function << "(";
	for (int i = 0; i < (int)m_arguments.size(); i++) {
		if (i)
			os << ",";
		os << *m_arguments[i];
	}
	return os << ")";
}

llvm::Value* CallExpression::codegen() {
	llvm::Value *fsv = m_function->codegen();
	FunctionType *ft = dynamic_cast<FunctionType*>(m_function->type());
	if (!ft)
		return ErrorV("This is not a function!");
	m_type = ft->returnType();
	vector<llvm::Value*> ids;
	ids.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)));
	ids.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,0,true)));
	llvm::Value *cv = builder.CreateGEP(fsv, ids);
	cv = builder.CreateLoad(cv, "closure");
	ids.pop_back();
	ids.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32,1,true)));
	llvm::Value *fv = builder.CreateGEP(fsv, ids);
	fv = builder.CreateLoad(fv, "functionpointer");
	vector<llvm::Value*> avs;
	avs.push_back(cv);
	for (Expression *e : m_arguments)
		avs.push_back(e->codegen());
	vector<Type*> ats = ft->argTypes();
	if (m_arguments.size() != ats.size())
		return ErrorV("Wrong number of arguments!");
	for (int i = 0; i < (int)m_arguments.size(); i++) {
		if (!(*m_arguments[i]->type() == *ats[i]))
			return ErrorV("Wrong argument type!");
	}
	return builder.CreateCall(fv, avs, "returnvalue");
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
	variables.push(map<string,vector<Variable*> >());
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
