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


#include "type.h"
#include "expression.h"

#include "llvm/LLVMContext.h"

Type::Type() {
	
}

Type::~Type() {
	
}

IntegerType::IntegerType() {

}

IntegerType::~IntegerType() {

}

ostream& IntegerType::print(ostream& os) const {
	return os << "int";
}

bool IntegerType::operator==(const Type& t) const {
	return dynamic_cast<const IntegerType*>(&t) != 0;
}

llvm::Type* IntegerType::codegen() {
	return llvm::Type::getInt32Ty(llvm::getGlobalContext());
}

ArrayType::ArrayType(Type* elementType) {
	m_elementType = elementType;
}

ArrayType::~ArrayType() {

}

ostream& ArrayType::print(ostream& os) const {
	return os << *m_elementType << "*";
}

bool ArrayType::operator==(const Type& t) const {
	const ArrayType *tt = dynamic_cast<const ArrayType*>(&t);
	if (!tt)
		return false;
	return m_elementType == tt->m_elementType;
}

llvm::Type* ArrayType::codegen() {
	return llvm::PointerType::get(m_elementType->codegen(),0);
}

Type* ArrayType::elementType() {
	return m_elementType;
}

FunctionType::FunctionType(Type* returnType, const vector< Type* >& argTypes) {
	m_returnType = returnType;
	m_argTypes = argTypes;
}

FunctionType::~FunctionType() {

}

ostream& FunctionType::print(ostream& os) const {
	os << *m_returnType << "(";
	for (int i = 0; i < (int)m_argTypes.size(); i++) {
		if (i)
			os << ",";
		os << *m_argTypes[i];
	}
	return os << ")";
}

bool FunctionType::operator==(const Type& t) const {
	const FunctionType *tt = dynamic_cast<const FunctionType*>(&t);
	if (!tt)
		return false;
	return m_returnType == tt->m_returnType && m_argTypes == tt->m_argTypes;
}

llvm::Type* FunctionType::codegen() {
	llvm::Type *rt = m_returnType->codegen();
	vector<llvm::Type*> ats;
	for (Type *t : m_argTypes)
		ats.push_back(t->codegen());
	llvm::FunctionType* ft = llvm::FunctionType::get(rt, ats, false);
	return llvm::PointerType::get(ft, 0);
}

ostream& operator<<(ostream& os, const Type& t) {
	os << "(";
	t.print(os);
	return os << ")";
}
