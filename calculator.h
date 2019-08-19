#pragma once

#include "calcatom.h"
#include <string>
#include <queue>

const unsigned char FUNC_ARG_SEPARATOR = ',';

class calc
{
public:
	enum mode { prefix, infix, postfix, converter };
	enum format { bin = 2, oct = 8, dec = 10, hex = 16 };

	/* PRE:
	* POST: returns pointer to calc object
	* USES:
	* THROWS:
	*/
	static calc *access();

	void setConvertTo(format to);
	void setConvertFrom(format from);

	/* PRE:
	* POST: solution to the expression or an error if expression is improper
	* USES:
	* THROWS:
	*/
	std::string solve(std::string expr) const;

	/* PRE:
	* POST: new calculator mode is set
	* USES:
	* THROWS: MODE_ERROR
	*/
	void setMode(enum mode type);

	/* PRE:
	* POST: current calculator mode is returned
	* USES:
	* THROWS:
	*/
	mode getMode() const;

private:
	calc();
	~calc();
	calc& operator=(const calc &other);
	calc(const calc &other);
	std::queue<Atom*> shuntingYardAlg(std::string expr) const;
	std::queue<Atom*> prefixMode(std::string expr) const;
	std::queue<Atom*> postfixMode(std::string expr) const;
	std::string eval(std::queue<Atom*> expr) const;
	std::string convert(std::string expr) const;
	Operator* isOperator(unsigned char expr) const;
	Operand operate(const Operand &op1, const Operand &op2, ops op) const;
	double getDigit(std::string str, std::size_t &iter) const;
	bool isVariable(unsigned char var) const;
	Function* isFunction(const std::string expr, std::size_t &iter) const;
	static unsigned char lowerCase(unsigned char ch);

	/* PRE: newFormat and oldFormat are correct values, expr is properly constructed expression of its base, newFormat != oldFormat
	* POST: expr is converted from base oldFormat to newFormat
	* USES: self recursive
	* THROWS:
	*/
	static std::string convertFormat(format newFormat, std::string expr, format oldFormat);

	static calc* m_Exists;
	mode m_Mode;
	format m_To;
	format m_From;
};