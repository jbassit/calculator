#include "stdafx.h"
#include "calculator.h"
#include <cstdlib>
#include <iostream>
#include <stack>
#include <regex>
#include <algorithm>

using namespace std;

const string PAR_ERROR = "Error: Mismatched parenthesis";
const string OP_ERROR = "Error: Undefined operator or function call";
const string CMD_ERROR = "Error: Unknown command or bad expression";
const string GEN_ERROR = "Error: Uexpected problem during calculation";
const string MODE_ERROR = "Error: Unknown mode enumerator";
const string E_ERROR = "Error: e is reserved and must not be used as regular variable";
const string FUNC_PARSE_ERROR = "Error: Failed an attempt at parsing function name";
const string FUNC_ARG_ERROR = "Error: Misplaced function argument separator or mismatched parenthesis";
const string FUNC_ARG_LIST_ERROR = "Error: Argument list compiled but never used, bad function name?";

const string DEC = "dec";
const string BIN = "bin";
const string OCT = "oct";
const string HEX = "hex";

calc* calc::m_Exists = NULL;

calc::calc()
	: m_Mode(infix), m_To(dec), m_From(dec)
{}

calc::~calc()
{
	if (m_Exists)
		delete m_Exists;
}

calc::calc(const calc &other)
{}

calc& calc::operator=(const calc &other)
{
	return *this;
}

calc* calc::access()
{
	if (!m_Exists)
		m_Exists = new calc;

	return m_Exists;
}

calc::mode calc::getMode() const
{
	return m_Mode;
}

Operator* calc::isOperator(unsigned char expr) const
{
	switch (expr)
	{
	case '^':
		return new Operator(POW);
	case '*':
		return new Operator(MULT);
	case '/':
		return new Operator(DIV);
	case '+':
		return new Operator(ADD);
	case '-':
		return new Operator(SUB);
	case '(':
		return new Operator(OPENP);
	case ')':
		return new Operator(CLOSEP);
	case '%':
		return new Operator(MODULO);
	case ',':
		return new Operator(COMMA);
	default:
		return NULL;
	}
}

bool calc::isVariable(unsigned char var) const
{
	if (((var >= 'a' && var <= 'z') || (var >= 'A' && var <= 'Z')) && var != 'e' && var != 'E')
		return true;

	if (var == 'e' || var == 'E')
		throw E_ERROR;

	return false;
}

Function* calc::isFunction(const string expr, size_t &iter) const
{
	regex parseFuncName("^([a-z]+)\\(.+");
	smatch parsedFunc;
	string funcName, temp = expr.substr(iter);

	if (regex_match(temp, parsedFunc, parseFuncName) && Function::functionExists(parsedFunc[1])) {
		funcName = parsedFunc[1];
		iter += funcName.size();
		return new Function(funcName);
	}
	return NULL;
}

queue<Atom*> calc::shuntingYardAlg(string expr) const
{
	stack<Atom*> OperatorStack;
	queue<Atom*> Output;
	string var;
	Operator *CurrentOperator;
	Function *CurrentFunc;
	for (size_t iter = 0; iter < expr.size();)
	{
		if (isdigit(expr[iter])) {
			if (iter > 0 && expr[iter - 1] == ')')
				expr.insert(iter, "*");
			else {
				Output.push(new Operand(getDigit(expr, iter)));
				if (isVariable(expr[iter]) || expr[iter] == '(')
					expr.insert(iter, "*");
			}
		}
		else
			if (isVariable(expr[iter]))
			{
				if ((CurrentFunc = isFunction(expr, iter)))
					OperatorStack.push(CurrentFunc);
				else {
					if (iter > 0 && expr[iter - 1] == ')')
						expr.insert(iter, "*");
					else
					{
						var = expr[iter];
						Output.push(new Operand(var));
						if (iter + 1 < expr.size() && (isdigit(expr[iter + 1]) || isVariable(expr[iter + 1]) || expr[iter + 1] == '('))
							expr.insert(iter + 1, "*");
						iter++;
					}
				}
			}
			else
				if ((CurrentOperator = isOperator(expr[iter])))
				{
					if (CurrentOperator->getType() == COMMA) {
						while (!OperatorStack.empty() && OperatorStack.top()->getId() != "(") {
							Output.push(OperatorStack.top());
							OperatorStack.pop();
						}
						if (OperatorStack.empty())
							throw FUNC_ARG_ERROR;
						Output.push(CurrentOperator);
					}
					else
						if (CurrentOperator->getType() == SUB && (iter == 0 || expr[iter - 1] == '(') && iter + 1 < expr.size()
							&& (expr[iter + 1] == '(' || isdigit(expr[iter + 1]) || isVariable(expr[iter + 1])))
						{
							Output.push(new Operand(0));
							OperatorStack.push(CurrentOperator);
						}
						else
							if (CurrentOperator->getType() != OPENP && CurrentOperator->getType() != CLOSEP)
							{
								while (!OperatorStack.empty() && OperatorStack.top()->getId() != "(" && OperatorStack.top()->getId() != ")"
									&& ((dynamic_cast<Operator*>(OperatorStack.top()) && CurrentOperator->isLeftAssociative()
										&& CurrentOperator->getPresedence() <= dynamic_cast<Operator*>(OperatorStack.top())->getPresedence())
										|| (dynamic_cast<Operator*>(OperatorStack.top()) && !CurrentOperator->isLeftAssociative()
											&& CurrentOperator->getPresedence() < dynamic_cast<Operator*>(OperatorStack.top())->getPresedence())))
								{
									Output.push(OperatorStack.top());
									OperatorStack.pop();
								}

								OperatorStack.push(CurrentOperator);
							}
							else
								if (CurrentOperator->getType() == OPENP)
									if (iter > 0 && (expr[iter - 1] == ')')) {
										expr.insert(iter, "*");
										iter--; // so that iterator stays in same position for next iteration
									}
									else
										OperatorStack.push(CurrentOperator);
								else
									if (CurrentOperator->getType() == CLOSEP)
									{
										while (!OperatorStack.empty() && OperatorStack.top()->getId() != "(")
										{
											Output.push(OperatorStack.top());
											OperatorStack.pop();
										}

										if (!OperatorStack.empty() && OperatorStack.top()->getId() == "(")
										{
											delete OperatorStack.top();
											OperatorStack.pop();
										}
										else
											throw PAR_ERROR;

										if (!OperatorStack.empty() && dynamic_cast<Function*>(OperatorStack.top())) {
											Output.push(OperatorStack.top());
											OperatorStack.pop();
										}
									}
					iter += CurrentOperator->getId().length();
				}
				else
					if (expr[iter] == ' ')
						iter++;
				else
					throw OP_ERROR;
	}

	while (!OperatorStack.empty())
	{
		if (OperatorStack.top()->getId() != "(" && OperatorStack.top()->getId() != ")")
			Output.push(OperatorStack.top());
		else
			throw PAR_ERROR;
		OperatorStack.pop();
	}

	return Output;
}

string calc::eval(queue<Atom*> expr) const
{
	stack<Operand*> OpStack;
	vector<Operand*> ArgList;
	Function* fnc;
	Operand *rightOp = NULL;
	Operator *CurrentOp = NULL;
	string result;

	if (expr.empty())
		return "";

	while (!expr.empty())
	{
		if ((CurrentOp = dynamic_cast<Operator*>(expr.front())))
		{
			if (CurrentOp->getType() == COMMA) {
				if (!OpStack.empty()) {
					ArgList.push_back(OpStack.top());
					OpStack.pop();
				}
				else
					throw CMD_ERROR;
			}
			else {
				if (!OpStack.empty())
				{
					rightOp = OpStack.top();
					OpStack.pop();
				}
				else
					throw CMD_ERROR;

				if (!OpStack.empty())
				{
					*rightOp = operate(*OpStack.top(), *rightOp, CurrentOp->getType());
					delete OpStack.top();
					OpStack.pop();
					OpStack.push(rightOp);
				}
				else
					throw CMD_ERROR;
			}

			delete CurrentOp;
			expr.pop();
		}
		else
			if (dynamic_cast<Operand*>(expr.front()))
			{
				OpStack.push(dynamic_cast<Operand*>(expr.front()));
				expr.pop();
			}
			else
				if ((fnc = dynamic_cast<Function*>(expr.front()))) {
					if (!OpStack.empty()) {
						ArgList.push_back(OpStack.top());
						OpStack.pop();
					}
					else
						throw CMD_ERROR;

					fnc->call(ArgList);
					OpStack.push(ArgList[0]);

					for (size_t i = 1; i < ArgList.size(); i++)
						delete ArgList[i];
					ArgList.clear();
					expr.pop();
				}
				else
					throw CMD_ERROR;
	}
	if (!ArgList.empty())
		throw FUNC_ARG_LIST_ERROR;

	if (OpStack.size() == 1)
	{
		result = OpStack.top()->getId();
		delete OpStack.top();
		return result;
	}
	else
		return GEN_ERROR;
}

queue<Atom*> calc::postfixMode(string expr) const
{
	queue<Atom*> Output;
	string var;
	Operator *CurrentOperator;

	for (size_t iter = 0; iter < expr.length();)
	{
		if (isdigit(expr[iter]))
			Output.push(new Operand(getDigit(expr, iter)));
		else
			if (isVariable(expr[iter]))
			{
				var = expr[iter];
				Output.push(new Operand(var));
				iter++;
			}
			else
				if ((CurrentOperator = isOperator(expr[iter])))
				{
					Output.push(CurrentOperator);
					iter += CurrentOperator->getId().length();
				}
				else
					if (expr[iter] == ' ')
						iter++;
					else
						throw OP_ERROR;
	}

	return Output;
}

queue<Atom *> calc::prefixMode(string expr) const
{
	queue<Atom*> STUB;
	return STUB;
}

double calc::getDigit(string str, size_t &iter) const
{
	string digit, temp = str.substr(iter);
	regex parseDigit("^([0-9]+\\.?[0-9]*(e.)?[0-9]*).*");
	regex assertInt("^[0-9]+$");
	regex assertDouble("^[0-9]+\\.[0-9]+$");
	regex assertScientific("^[0-9]+e(\\+|\\-)[0-9]{1,3}$");
	regex assertScientificDouble("^[0-9]+\\.[0-9]+e(\\+|\\-)[0-9]{1,3}$");
	smatch parsedDigit;

	if (regex_match(temp, parsedDigit, parseDigit))
		digit = parsedDigit[1];
	else
		throw CMD_ERROR;

	if (regex_match(digit, assertInt) || regex_match(digit, assertDouble)
		|| regex_match(digit, assertScientific) || regex_match(digit, assertScientificDouble))
		iter += digit.size();
	else
		throw CMD_ERROR;

	if (str[iter] == '.')
		throw CMD_ERROR;

	return strtod(digit.c_str(), NULL);
}

Operand calc::operate(const Operand &op1, const Operand &op2, ops op) const
{
	switch (op)
	{
	case POW:
		return Operand::power(op1, op2);
	case MULT:
		return op1*op2;
	case DIV:
		return op1 / op2;
	case ADD:
		return op1 + op2;
	case SUB:
		return op1 - op2;
	case MODULO:
		return op1%op2;
	default:
		throw OP_ERROR;
	}
}

void calc::setMode(mode type)
{
	if (type != m_Mode)
	{
		switch (type)
		{
		case prefix:
			m_Mode = prefix;
			break;
		case infix:
			m_Mode = infix;
			break;
		case postfix:
			m_Mode = postfix;
			break;
		case converter:
			m_Mode = converter;
			break;
		default:
			throw MODE_ERROR;
		}
	}
}

unsigned char calc::lowerCase(unsigned char ch)
{
	if (ch <= 'Z' && ch >= 'A')
		return ch - ('A' - 'a');
	return ch;
}

string calc::convert(string expr) const {
	regex assertBin("[0-1]+");
	regex assertOct("[0-7]+");
	regex assertDec("[0-9]+");
	regex assertHex("(0x)?[a-f0-9]+");
	string result;

	if ((m_From == bin && !regex_match(expr, assertBin))
		|| (m_From == oct && !regex_match(expr, assertOct))
		|| (m_From == dec && !regex_match(expr, assertDec))
		|| (m_From == hex && !regex_match(expr, assertHex)))
		throw CMD_ERROR;
	if (m_From == hex && expr[0] == '0' && expr[1] == 'x')
		expr = expr.substr(2);

	if (m_From == m_To)
		result = expr;
	else
		result = convertFormat(m_To, expr, m_From);

	return result;
}

string calc::convertFormat(format newFormat, string expr, format oldFormat) {
	string result = "0", tempS;
	unsigned int sum = 0, temp, digitCount = 0;

	switch (newFormat) {
	case dec:
		for (long i = expr.size() - 1, k = 0; i >= 0; i--, k++) {
			if (expr[i] > '9')
				sum += (expr[i] - 'a' + 10) * (unsigned int)pow((int)oldFormat, k);
			else
				sum += (expr[i] - '0') * (unsigned int)pow((int)oldFormat, k);
		}
		break;
	case oct:
	case bin:
		result = "";
		if (oldFormat != dec)
			expr = convertFormat(dec, expr, oldFormat);

		sum = stoi(expr);
		while (sum) {
			result = to_string(sum%newFormat) + result;
			sum = sum / newFormat;
			digitCount++;
			if ((digitCount % 4) == 0 && sum)
				result = " " + result;
		}
		break;
	case hex:
		result = "";
		if (oldFormat != dec)
			expr = convertFormat(dec, expr, oldFormat);

		sum = stoi(expr);
		while (sum) {
			temp = sum%hex;
			if (temp > 9) {
				tempS.push_back('a' + temp - 10);
				result = tempS + result;
				tempS = "";
			}
			else
				result = to_string(temp) + result;
			sum = sum / hex;
			digitCount++;
			if ((digitCount % 4) == 0 && sum)
				result = " " + result;
		}
	}
	while ((digitCount % 4) != 0)
	{
		result = "0" + result;
		digitCount++;
	}

	if (sum)
		return to_string(sum);
	else
		return result;
}

string calc::solve(string expr) const
{
	string Input = "";

	transform(expr.begin(), expr.end(), expr.begin(), lowerCase);
	try
	{
		switch (m_Mode)
		{
		case infix:
			Input = eval(shuntingYardAlg(expr));
			break;
		case postfix:
			Input = eval(postfixMode(expr));
			break;
		case prefix:
			Input = eval(prefixMode(expr));
			break;
		case converter:
			Input = convert(expr);
			break;
		default:
			throw MODE_ERROR;
		}
	}
	catch (const string ErrMsg)
	{
		Input = ErrMsg;
	}

	return Input;
}

void calc::setConvertTo(format to) {
	m_To = to;
}

void calc::setConvertFrom(format from) {
	m_From = from;
}