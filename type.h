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


#ifndef TYPE_H
#define TYPE_H

#include "llvm/DerivedTypes.h"

#include <vector>
#include <ostream>
using namespace std;

class Type {
public:
	Type();
	virtual ~Type();
	virtual ostream& print(ostream& os) const = 0;
	virtual bool operator==(const Type &t) const = 0;
	virtual llvm::Type* codegen() = 0;
};

class IntegerType : public Type {
public:
	IntegerType();
	virtual ~IntegerType();
	virtual ostream& print(ostream& os) const;
	virtual bool operator==(const Type& t) const;
	virtual llvm::Type* codegen();
};

class ArrayType : public Type {
public:
	ArrayType(Type *elementType);
	virtual ~ArrayType();
	virtual ostream& print(ostream& os) const;
	virtual bool operator==(const Type& t) const;
	virtual llvm::Type* codegen();
	Type *elementType();
private:
	Type *m_elementType;
};

class FunctionType : public Type {
public:
	FunctionType(Type *returnType, const vector<Type*> &argTypes);
	virtual ~FunctionType();
	virtual ostream& print(ostream& os) const;
	virtual bool operator==(const Type& t) const;
	llvm::FunctionType* functionType();
	virtual llvm::Type* codegen();
	Type *returnType();
	vector<Type*> argTypes();
private:
	Type *m_returnType;
	vector<Type*> m_argTypes;
};

ostream & operator<<(ostream &os, const Type &t);

#endif // TYPE_H
