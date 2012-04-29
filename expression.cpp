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

Expression::Expression() {

}

Expression::~Expression() {

}

NumberExpression::NumberExpression(int number) {
	m_number = number;
}

NumberExpression::~NumberExpression() {

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

ostream& operator<<(ostream& os, const Expression& e) {
	os << "(";
	e.print(os);
	return os << ")";
}
