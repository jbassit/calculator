#include "stdafx.h"
#include "calcatom.h"
#include <sstream>
#include <math.h>
#include <cstdlib>
#include <iostream>
using namespace std;

const string DIV_ZERO_ERROR = "Error: Division by zero";
const string BAD_FACT_ERROR = "Error: Can only factorial whole numbers > 0. "
"This condition must be handled at higher level";
const string BAD_ADD_SUB_ERROR = "Error: Operands must have same variables in order to be "
"added or subtracted. This condition must be handled at higher level";
const string BAD_BASE_ERROR = "Error: Base must be > 1";
const string BASE_ERROR = "Error: Both expressions must have same base";
const string BAD_OP_ALLOC = "Error: Operand was not allocated properly";
const string BAD_VAR_SIZE = "Error: Number of variables must match the number of their exponents";
const string BAD_VAR_NAME = "Error: Variables must be unique";
const string BAD_VAR_EXP = "Error: Variable must not have exponent of 0";
const string BAD_GROUP = "Error: Operand expected to have an empty group";
const string BAD_EXP_COMP = "Error: For comparison both exponents must be either NULL or NON-NULL";
const string BAD_TYPE_ERROR = "Error: Unknown operator type";
const string MODULO_ERROR = "Error: Both operands must be non-negative integers";
const string UNKNOWN_FUNCTION = "Error: Function does not exist";

const string BAD_ARG_SIZE = "Error: Bad argument size";
const string LOG_ERROR = "Error: Argument must be a non negative real number";

bool Operand::m_Combined = false;

static map<string, void(*)(vector<Operand*>&)> fncInit() { // initializes functions map with pre defined functions
	map<string, void(*)(vector<Operand*>&)> temp;
	temp[LOG_FNC] = &log10_f;
	temp[LN_FNC] = &ln_f;
	temp[RECT_PRISM_AREA] = &rectPrismArea_f;

	return temp;
}

map<string, void(*)(vector<Operand*>&)> Function::functions = fncInit();

Operand::Operand()
	: m_Value(DEFAULT_VALUE), m_GlobalExp(NULL), m_Base(DEFAULT_BASE), m_CustomDesc("")
{}

Operand::Operand(double value, unsigned base)
	: m_Value(value), m_GlobalExp(NULL), m_Base(base), m_CustomDesc("")
{
	if (m_Base < 2) // base check
		throw BAD_BASE_ERROR;
}

Operand::Operand(string variable, unsigned base)
	: m_Value(1), m_GlobalExp(NULL), m_Base(base), m_CustomDesc("")
{
	if (m_Base < 2) // base check
		throw BAD_BASE_ERROR;

	if (variable.size() != 1) // ensure that only single variable is present
		throw BAD_VAR_NAME;

	if (variable[0] < 'A' || variable[0] > 'z' || (variable[0] > 'Z' && variable[0] < 'a')) // check that variable is within accepted range
		throw BAD_VAR_NAME;

	if (variable[0] < 'a') // convert to lower case
		variable[0] += 'a' - 'A';

	m_Variables.push_back(variable);
	m_VarExponent.push_back(new Operand(DEFAULT_EXPONENT));
}

Atom::~Atom()
{}

Operand::~Operand()
{
	clearOpList(m_VarExponent);
	clearOpList(m_Group);

	if (m_GlobalExp)
		delete m_GlobalExp;
}

void Operand::clearOpList(std::list<Operand*> &list)
{
	while (!list.empty())
	{
		if (list.back())
			delete list.back();
		list.pop_back();
	}
}

bool Operand::combExprOnly() const
{
	for (list<string>::const_iterator variter = m_Variables.begin(); variter != m_Variables.end(); ++variter)
		if (*variter != COMBINATION_TOKEN)
			return false;

	return true;
}

bool Operand::containsCombToken() const
{
	for (list<string>::const_iterator variter = m_Variables.begin(); variter != m_Variables.end(); ++variter)
		if (*variter == COMBINATION_TOKEN)
			return true;

	return false;
}

bool Operand::singleGroupExpr() const
{
	for (list<Operand*>::const_iterator groupiter = m_Group.begin(); groupiter != m_Group.end(); ++groupiter)
		if ((*groupiter)->m_GlobalExp)
			return false;

	return true;
}
bool Operand::sameVariables(const Operand &op1, const Operand &op2)
{
	list<Operand*>::const_iterator op1expiter = op1.m_VarExponent.begin();
	list<Operand*>::const_iterator op2expiter = op2.m_VarExponent.begin();
	bool Match = false;

	if (op1.m_Variables.size() != op2.m_Variables.size()) // no need to compare if operands have different number of variables
		return false;

	for (list<string>::const_iterator op1iter = op1.m_Variables.begin(); op1iter != op1.m_Variables.end(); ++op1iter, ++op1expiter)
	{
		for (list<string>::const_iterator op2iter = op2.m_Variables.begin(); op2iter != op2.m_Variables.end() && !Match; ++op2iter, ++op2expiter)
			if ((*op1iter == *op2iter) && (**op1expiter == **op2expiter)) // checks if both variables are identical
				Match = true;

		if (!Match) // no point to continue if there exists unique variable
			return false;
		Match = false;
		op2expiter = op2.m_VarExponent.begin(); // resets variable exponent iterator
	}
	return true;
}

bool Operand::sameGroupTwo(const Operand &op1, const Operand &op2)
{
	Operand temp1 = op1;
	Operand temp2 = op2;
	list<Operand*> list1 = op1.m_Group; // extracts all sums from first operand
	list<Operand*> list2 = op2.m_Group; // extracts all sums from second operand
	bool Match = false;
	temp1.clearGroup(); // clears all sums from first operand
	temp2.clearGroup(); // clears all sums from second operand

	if (op1.m_GlobalExp && op2.m_GlobalExp) // checks for existence of global exponents for both operands
	{
		delete temp1.m_GlobalExp; // removes global exponent from first operand
		delete temp2.m_GlobalExp; // removes global exponent from second operand
		temp1.m_GlobalExp = NULL;
		temp2.m_GlobalExp = NULL;
	}
	else
		if (op1.m_GlobalExp || op2.m_GlobalExp) // two global exponents cannot possibly be same in this case
			return false;

	list1.push_front(&temp1);
	list2.push_front(&temp2);

	if (list1.size() != list2.size()) // makes sure that number of operands is the same in both lists
		return false;

	for (list<Operand*>::iterator iter1 = list1.begin(); iter1 != list1.end(); iter1++)
	{
		for (list<Operand*>::iterator iter2 = list2.begin(); iter2 != list2.end() && !Match; iter2++)
			if (**iter1 == **iter2) // attempts to map identical operands from both lists
				Match = true;

		if (!Match) // returns false if a unique operand was found
			return false;
		Match = false;
	}

	return true;
}

bool Operand::sameGroup(const Operand &op1, const Operand &op2)
{
	list<Operand*> grp1;
	list<Operand*> grp2;
	Operand temp1 = op1;
	Operand temp2 = op2;
	bool Match = false;
	temp1.clearGlobalExpCombGroup();
	temp2.clearGlobalExpCombGroup();

	temp1.m_Value = 1;
	temp2.m_Value = 1;

	if (!containsCombToken(op1) && !containsCombToken(op2)) // case where neither operands contain combinatorial tokens
	{
		if (!op1.m_Group.empty() && !op2.m_Group.empty())
			return sameGroupTwo(op1, op2);
		else
			if (op1.m_Group.empty() && op2.m_Group.empty())
				return sameVariables(op1, op2);
			else
				return false;
	}

	grp1 = getCombExprs(op1);
	grp2 = getCombExprs(op2);
	grp1.push_front(new Operand(temp1));
	grp2.push_front(new Operand(temp2));

	if (grp1.size() != grp2.size())
		return false;

	for (list<Operand*>::const_iterator op1iter = grp1.begin(); op1iter != grp1.end(); ++op1iter)
	{
		for (list<Operand*>::const_iterator op2iter = grp2.begin(); op2iter != grp2.end() && !Match; ++op2iter)
			if (**op1iter == **op2iter) // attempts to map identical combinatorial groups from both lists
				Match = true;

		if (!Match) // returns false if unique combinatorial group is found
			return false;
		Match = false;
	}

	return true;
}

bool Operand::containsCombToken(const Operand &op)
{
	for (list<string>::const_iterator variter = op.m_Variables.begin(); variter != op.m_Variables.end(); ++variter)
		if (*variter == COMBINATION_TOKEN)
			return true;

	return false;
}

bool Operand::sameGlobalExponent(const Operand *const exp1, const Operand *const exp2)
{
	if ((!exp1 && exp2) || (exp1 && !exp2)) // structural consistency issue if this statement ever returns true
		throw BAD_EXP_COMP;

	if (exp1 == exp2 || *exp1 == *exp2)
		return true;

	return false;
}

bool Operand::canAdd(const Operand &op1, const Operand &op2)
{
	if (sameGroup(op1, op2) && sameGlobalExponent(op1.m_GlobalExp, op2.m_GlobalExp) && op1.m_Base == op2.m_Base)
		return true;

	return false;
}

Operand::Operand(const Operand &other)
	: m_Value(other.m_Value), m_Base(other.m_Base)
{
	m_Variables = other.m_Variables;

	if (other.m_GlobalExp)
		m_GlobalExp = new Operand(*other.m_GlobalExp);
	else
		m_GlobalExp = NULL;

	copyOpList(m_VarExponent, other.m_VarExponent);
	copyOpList(m_Group, other.m_Group);

	if (m_Variables.size() != m_VarExponent.size()) // should never possible, just a safety check
		throw BAD_VAR_SIZE;
}

bool Operand::operator==(const Operand &other) const
{
	if (this == &other)
		return true;

	if (m_Value != other.m_Value || m_Base != other.m_Base || !sameGroup(*this, other)
		|| !sameGlobalExponent(m_GlobalExp, other.m_GlobalExp))
		return false;

	return true;
}

bool Operand::operator!=(const Operand &other) const
{
	if (*this == other)
		return false;

	return true;
}

bool Operand::operator!=(const double value) const
{
	if (*this == value)
		return false;

	return true;
}

bool Operand::operator==(const double value) const
{
	if (m_Group.empty() && m_Variables.empty() && m_Base == DEFAULT_BASE && m_Value == value
		&& !m_GlobalExp)
		return true;

	return false;
}

void Operand::copyOpList(std::list<Operand *> &dest, const std::list<Operand *> &source)
{
	for (list<Operand*>::const_iterator iter = source.begin(); iter != source.end(); ++iter)
		dest.push_back(new Operand(**iter));
}

Operand& Operand::operator=(const Operand &other)
{
	if (this != &other)
	{
		m_Variables.clear();
		clearOpList(m_VarExponent);
		clearOpList(m_Group);

		if (m_GlobalExp)
			delete m_GlobalExp;

		m_Value = other.m_Value;
		m_Base = other.m_Base;
		m_Variables = other.m_Variables;

		if (other.m_GlobalExp)
			m_GlobalExp = new Operand(*other.m_GlobalExp);
		else
			m_GlobalExp = NULL;

		if (m_Variables.size() == other.m_VarExponent.size())
			copyOpList(m_VarExponent, other.m_VarExponent);
		else // should never possible, just a safety check
			throw BAD_VAR_SIZE;

		copyOpList(m_Group, other.m_Group);
	}
	return *this;
}
Operand& Operand::operator=(double value)
{
	m_Value = value;
	m_Variables.clear();
	clearOpList(m_VarExponent);
	clearOpList(m_Group);

	if (m_GlobalExp)
		delete m_GlobalExp;
	m_GlobalExp = NULL;

	return *this;
}

void Operand::simpZero(Operand &op)
{
	list<Operand*> tempgroup;

	if (op.m_Value == 0) // checks if zero is at the head of the sum
	{
		if (op.m_Group.empty())
			op = 0;
		else // removes zero from the sum
		{
			copyOpList(tempgroup, op.m_Group); // saves original group
			op = *tempgroup.front(); // makes head of original group also head of the sum
			delete tempgroup.front(); // removes head of original group
			tempgroup.pop_front();
			op.m_Group = tempgroup; // assigns original group to new head of the sum
		}
	}

	list<Operand*>::iterator iter = op.m_Group.begin();
	while (iter != op.m_Group.end()) // simplifies rest of the sum in case there are more 0's
	{
		if ((*iter)->m_Value == 0)
		{
			delete *iter;
			iter = op.m_Group.erase(iter);
		}
		else
			++iter;
	}
}

void Operand::adjustComplex(std::list<Operand*> &varexp, std::list<Operand*>::iterator &varexppos
	, std::list<std::string> &variables, std::list<std::string>::iterator &varpos) // garbage function. needs to be completely re-done
{
	if (((*varexppos)->m_Value - floor((*varexppos)->m_Value)) == 0)
	{
		(*varexppos)->m_Value = (int)(*varexppos)->m_Value % 4;
		if ((*varexppos)->m_Value == 2 || (*varexppos)->m_Value == 0)
		{
			if ((*varexppos)->m_Value == 2)
				m_Value = 0 - m_Value;
			varexp.erase(varexppos);
			variables.erase(varpos);
			delete *varexppos;
		}
	}
}

bool Operand::validateDigit(const string &digit)
{
	bool Decimal = false;

	for (unsigned i = 0; i < digit.size(); i++) // iterates through all characters
	{
		if (!isdigit(digit[i]) && !(digit[i] == '-' && i == 0) && !(digit[i] == '.' && !Decimal && i + 1 < digit.size() && int(i) - 1 >= 0))  // make sure character is part of a real number
			return false;

		if (digit[i] == '.') // assures that dot may only appear once in this string
			Decimal = true;
	}
	return true;
}

void Operand::clearGroup()
{
	list<Operand*>::iterator iter = m_Group.begin();

	while (iter != m_Group.end())
	{
		delete *iter;
		iter = m_Group.erase(iter);
	}
}

void Operand::clearGlobalExpGroup()
{
	list<Operand*>::iterator iter = m_Group.begin();

	while (iter != m_Group.end())
	{
		if ((*iter)->m_GlobalExp) // deletes operand from group if global exponent is present
		{
			delete *iter;
			iter = m_Group.erase(iter);
		}
		else
			++iter;
	}
}

void Operand::clearGlobalExpCombGroup()
{
	list<string>::iterator variter = m_Variables.begin();
	list<Operand*>::iterator iter = m_VarExponent.begin();
	while (iter != m_VarExponent.end())
	{
		if (*variter == COMBINATION_TOKEN) // deletes  variable and exponent if it is a combinatorial expression
		{
			variter = m_Variables.erase(variter);
			delete *iter;
			iter = m_VarExponent.erase(iter);
		}
		else
		{
			++iter;
			++variter;
		}
	}
}

Operand Operand::combineGlobalExpOps2(const Operand &left, const Operand &right)
{
	Operand result;

	if (left.m_GlobalExp && right.m_GlobalExp && sameGroup(left, right)) // branch here if both expressions are equivalent except for their NON-NULL global exponents
	{
		m_Combined = true;
		result = left;
		*result.m_GlobalExp = *left.m_GlobalExp + *right.m_GlobalExp;

		if (result.m_GlobalExp->m_Value == 1 && result.m_GlobalExp->m_Group.empty()
			&& result.m_GlobalExp->m_Variables.empty()) // remove global exponent if its value becomes 1
		{
			delete result.m_GlobalExp;
			result.m_GlobalExp = NULL;
		}
		if (result.m_GlobalExp->m_Value == 0) // case where global exponent value becomes 0
			return Operand(1);
	}
	else
		if (left.m_GlobalExp && right.m_GlobalExp && *left.m_GlobalExp == *right.m_GlobalExp) // case where body is different but global exponents are equivalent
		{
			Operand templeft = left;
			Operand tempright = right;
			delete templeft.m_GlobalExp;
			templeft.m_GlobalExp = NULL;
			delete tempright.m_GlobalExp;
			tempright.m_GlobalExp = NULL;

			result = templeft * tempright;
			result.m_GlobalExp = new Operand(*left.m_GlobalExp);
		}

	if (!left.m_GlobalExp && !right.m_GlobalExp) // attempts to recover from case where neither expressions have global exponents, this should be possible
		result = left * right;

	return result;
}

list<Operand*> Operand::getCombExprs(const Operand &op)
{
	list<Operand*> ls;

	list<string>::const_iterator variter = op.m_Variables.begin();
	for (list<Operand*>::const_iterator iter = op.m_VarExponent.begin(); iter != op.m_VarExponent.end(); ++iter, ++variter)
		if (*variter == COMBINATION_TOKEN) // adds reference of combinatorial expression to the list
			ls.push_back(*iter);

	return ls;
}

Operand Operand::combineGlobalExpOps(const Operand &op1, const Operand &op2)
{
	Operand result;
	Operand temp;
	list<Operand*> leftpart;
	list<Operand*> rightpart;
	bool listSimplified;
	Operand left = op1;
	Operand right = op2;
	m_Combined = false;

	if (!op1.m_GlobalExp) // breaks down expression into smaller components if no global exponent is present
	{
		leftpart = getCombExprs(op1);
		left.clearGlobalExpCombGroup();
	}
	if (!op2.m_GlobalExp)
	{
		rightpart = getCombExprs(op2);
		right.clearGlobalExpCombGroup();
	}

	rightpart.push_front(&right);
	leftpart.push_front(&left);

	do // simplifies product of two expressions for as long as there is something to simplify
	{
		listSimplified = false;
		list<Operand*>::iterator leftiter = leftpart.begin();

		while (leftiter != leftpart.end() && !listSimplified)
		{
			for (list<Operand*>::iterator rightiter = rightpart.begin(); rightiter != rightpart.end() && !listSimplified;)
			{
				if (((*leftiter)->m_Group.empty() || (*leftiter)->singleGroupExpr()) && ((*rightiter)->m_Group.empty() || (*rightiter)->singleGroupExpr()))
					temp = combineGlobalExpOps2(**leftiter, **rightiter); // attempts to take product of two expressions
				else // attempts to recover from should-be-impossible case
					temp = **leftiter * **rightiter;

				if (m_Combined) // branches here if product resulted in simplified expression
				{
					**leftiter = temp; // left side expression is replaced with its simplified counterpart
					rightiter = rightpart.erase(rightiter); // right side expression is removed
					listSimplified = true;
				}
				if (!listSimplified) {
					++rightiter;
				}
			}
			m_Combined = false;
			++leftiter;
		}
	} while (listSimplified);

	/*if no global exponent is present at the upper level of expression, then we can make it a starting point
	* of building new expression (note that front of the list will always contain upper level of an expression).
	* We cannot build a new expression if upper level contains global exponent because it will create ambiguity
	* in cases such as (a+b)^c*(b+c)^c vs ((a+b)^c + b)^d, in a former case presence of global exponent would not
	* make its effect "global" */
	if (!op1.m_GlobalExp || !op2.m_GlobalExp)
	{
		if (!op1.m_GlobalExp)
		{
			result = *leftpart.front(); // begins construction of resulting expression
			leftpart.pop_front();
		}
		else
		{
			result = *rightpart.front();
			rightpart.pop_front();
		}
	}

	for (list<Operand*>::iterator iter = leftpart.begin(); iter != leftpart.end(); ++iter) // appends all remaining expressions from left list
	{
		if (!(**iter == 1)) // ignore any simplifications that resulted in 1
		{
			result.m_Variables.push_back(COMBINATION_TOKEN);
			result.m_VarExponent.push_back(new Operand(**iter));
		}
	}
	for (list<Operand*>::iterator iter = rightpart.begin(); iter != rightpart.end(); ++iter) // appends all remaining expressions from right list
	{
		if (!(**iter == 1)) // ignore any simplifications that resulted in 1, although this check should not be needed on rightpart
		{
			result.m_Variables.push_back(COMBINATION_TOKEN);
			result.m_VarExponent.push_back(new Operand(**iter));
		}
	}

	return result;
}

Operand Operand::combineOps(const Operand &left, const Operand &right)
{
	double tempRoot, temp;
	unsigned short root = 2;
	unsigned short maxRoot = 0;
	bool uniquevar, removedItems, negative;
	stringstream strbuff;
	Operand result = left;

	result.clearGroup(); // removes rest of the sum, leaving just the head
	list<string>::const_iterator sourcevariter = right.m_Variables.begin();
	for (list<Operand*>::const_iterator sourceiter = right.m_VarExponent.begin(); //iterates through variables of source operand
		sourceiter != right.m_VarExponent.end(); ++sourceiter, ++sourcevariter)
	{
		uniquevar = true;
		list<string>::iterator destvariter = result.m_Variables.begin();
		for (list<Operand*>::iterator destiter = result.m_VarExponent.begin(); destiter != result.m_VarExponent.end() && uniquevar;) // iterates through variables of destination operand
		{
			removedItems = false;
			if (*destvariter == COMBINATION_TOKEN && *sourcevariter == COMBINATION_TOKEN) // case where variable is really another operand or a number with an exponent
			{
				**destiter = **sourceiter * **destiter;

				if (((*destiter)->m_Value) == 1 && (*destiter)->m_Variables.empty() && (*destiter)->m_Group.empty()) // remove operand that simplifies to 1
				{
					removedItems = true;
					delete *destiter;
					destiter = result.m_VarExponent.erase(destiter);
					destvariter = result.m_Variables.erase(destvariter);
				}

				uniquevar = false;
			}
			else
				if (*sourcevariter == *destvariter || (validateDigit(*sourcevariter) && validateDigit(*destvariter) && **sourceiter == **destiter)) // case where variables are same
				{
					if (validateDigit(*sourcevariter) && **sourceiter == **destiter) // case where "variable" is two real numbers with variable as an exponent
					{
						negative = false;
						temp = strtod(sourcevariter->c_str(), NULL) * strtod(destvariter->c_str(), NULL); // takes product of two numbers
						if (temp < 0) { // convert to positive value to find maxroot
							negative = true;
							temp = -1 * temp;
						}
						do // attempts to simplify real part by finding largest possible root
						{
							tempRoot = pow(temp, (double)1 / root);
							if ((tempRoot - floor(tempRoot)) == 0)
								maxRoot = root;
							root++;
						} while (tempRoot > 2); // square root is smallest possible root

						if (maxRoot != 0) // case where root was found
						{
							temp = pow(temp, (double)1 / maxRoot);
							**destiter = **destiter * maxRoot;
						}
						if (negative) { // convert back to negative
							temp = -1 * temp;
						}
						strbuff << temp;
						*destvariter = strbuff.str();
					}
					else // case with regular variables
						**destiter = **destiter + **sourceiter; // adds exponents

																//if(*destvariter == COMPLEX_TOKEN) // case where it is a complex number, garbage oode
																//     result.adjustComplex(result.m_VarExponent, destiter, result.m_Variables, destvariter);
																// else
					if (((*destiter)->m_Value) == 0 && (*destiter)->m_Group.empty()) // remove variable when exponent is 0
					{
						removedItems = true;
						delete *destiter;
						destiter = result.m_VarExponent.erase(destiter);
						destvariter = result.m_Variables.erase(destvariter);
					}

					uniquevar = false;
				}
			if (!removedItems) {
				++destiter;
				++destvariter;
			}
		}
		if (uniquevar) // case where variable is unique, added to list of variables
		{
			result.m_Variables.push_back(*sourcevariter);
			result.m_VarExponent.push_back(new Operand(**sourceiter));
		}
	}
	result.m_Value = left.m_Value * right.m_Value; // multiplies magnitudes

	return result;
}

Operand operator+(const Operand &op1, const Operand &op2)
{
	Operand left = 1 * op1; // multiplication by 1 is necessary to push down potential expression with global exponent as a combinatorial token, this allows add itentical expressions without ambiguity
	Operand right = 1 * op2;
	Operand result = left;
	bool Match = false;

	if (op1 == 0)
		return op2;
	if (op2 == 0)
		return op1;
	if (op1.m_Base != op2.m_Base)
		throw BASE_ERROR;

	left.clearGroup();
	right.clearGroup();

	if (Operand::canAdd(left, right)) // case where operands can be added
	{
		result.m_Value += right.m_Value;
		Match = true;
	}
	else
		for (list<Operand*>::iterator dest = result.m_Group.begin(); dest != result.m_Group.end() && !Match; dest++) // attempt to add head of right operand to any part of left operand
		{
			if (Operand::canAdd(1 * **dest, right)) // checks if addition is possible
			{
				(*dest)->m_Value += right.m_Value;
				Match = true;
			}
		}

	if (!Match)
	{
		if (!right.m_Group.empty()) // assertion that operand to be added to a group has no group of its own
			throw BAD_GROUP;
		result.m_Group.push_back(new Operand(right));
	}
	else
		Operand::simpZero(result); // simplifies any resulting 0's

	if (!op2.m_GlobalExp) // attempts to add remaining parts of right operand to left operand
		for (list<Operand*>::const_iterator rightiter = op2.m_Group.begin(); rightiter != op2.m_Group.end(); ++rightiter)
			result = result + **rightiter;

	return result;
}

Operand operator-(const Operand &left, const Operand &right)
{
	return left + -1 * right; // defines subtraction in terms of addition and multiplication
}

Operand operator-(const Operand &op)
{
	return -1 * op;
}

bool Operand::isProperInt() const {
	if (!m_GlobalExp && m_Group.empty() && m_Variables.empty()
		&& m_Base == DEFAULT_BASE && (m_Value - floor(m_Value)) == 0.0)
		return true;

	return false;
}

Operand operator%(const Operand &op1, const Operand &op2) {
	Operand result;

	if (op1.isProperInt() && op1.m_Value >= 0 && op2.isProperInt() && op2.m_Value > 0)
		result.m_Value = (int)op1.m_Value % (int)op2.m_Value;
	else
		throw MODULO_ERROR;

	return result;
}

Operand operator*(const Operand &op1, const Operand &op2)
{
	Operand result;
	Operand left, right;

	if (op1 == 0 || op2 == 0)
		return Operand(0);
	if (op1.m_Base != op2.m_Base)
		throw BASE_ERROR;

	if (op2.m_Group.empty() && op2.m_Variables.empty() && !op2.m_GlobalExp) // prefers number without exponent as left operand
	{
		left = op2;
		right = op1;
	}
	else
	{
		left = op1;
		right = op2;
	}

	if (left.m_Value == 0 || right.m_Value == 0)
		return Operand(0);

	if (!left.m_GlobalExp && !right.m_GlobalExp) // no global exponents are present
	{
		Operand tempright(right);
		Operand templeft(left);
		tempright.clearGroup();
		templeft.clearGroup();
		Operand::m_Combined = true;

		result = Operand::combineOps(templeft, tempright); // combine heads of both operands

														   // takes cross product
		for (list<Operand*>::const_iterator leftiter = left.m_Group.begin(); leftiter != left.m_Group.end(); ++leftiter)
			result = result + (**leftiter * tempright);

		for (list<Operand*>::const_iterator rightiter = right.m_Group.begin(); rightiter != right.m_Group.end(); ++rightiter)
		{
			result = result + (templeft * **rightiter);
			for (list<Operand*>::const_iterator leftiter = left.m_Group.begin(); leftiter != left.m_Group.end(); ++leftiter)
				result = result + (**leftiter * **rightiter);
		}
	}
	else
	{
		Operand tempright(right);
		Operand templeft(left);
		if (right.m_GlobalExp)
			tempright.clearGlobalExpGroup();
		else
			tempright.clearGroup();
		if (templeft.m_GlobalExp)
			templeft.clearGlobalExpGroup();
		else
			templeft.clearGroup();

		result = Operand::combineGlobalExpOps(templeft, tempright); // combines head of both operands

																	// takes potential cross product
		for (list<Operand*>::const_iterator leftiter = left.m_Group.begin(); leftiter != left.m_Group.end(); ++leftiter)
			if (!left.m_GlobalExp || (*leftiter)->m_GlobalExp)
				result = result + (**leftiter * tempright);

		for (list<Operand*>::const_iterator rightiter = right.m_Group.begin(); rightiter != right.m_Group.end(); ++rightiter)
		{
			if (!right.m_GlobalExp || (*rightiter)->m_GlobalExp)
				result = result + (templeft * **rightiter);
			for (list<Operand*>::const_iterator leftiter = left.m_Group.begin(); leftiter != left.m_Group.end(); ++leftiter)
				if ((!left.m_GlobalExp || (*leftiter)->m_GlobalExp) && (!right.m_GlobalExp || (*rightiter)->m_GlobalExp))
					result = result + (**leftiter * **rightiter);
		}
	}

	return result;
}

Operand operator/(const Operand &left, const Operand &right)
{
	if (right.m_Value == 0) // handles division by zero
		throw DIV_ZERO_ERROR;
	if (left.m_Value == 0)
		return Operand(0);
	if (left == right)
		return Operand();

	return left * Operand::power(right, -1); // defines division in terms of multiplication;
}

Operand Operand::power(const Operand &op, const Operand &power)
{
	Operand result(op.m_Value);
	Operand temp, tempPower;
	stringstream strbuff;

	if (op.m_Base != power.m_Base)
		throw BASE_ERROR;
	if (op.m_Value == 0 && op.m_Variables.empty() && op.m_Group.empty())
		return Operand(0);
	if ((op.m_Value == 1 && op.m_Variables.empty() && op.m_Group.empty()) || power.m_Value == 0)
		return Operand();
	if (power.m_Value == 1 && power.m_Variables.empty() && power.m_Group.empty())
		return op;

	if (!power.m_GlobalExp && !op.m_GlobalExp && power.m_Group.empty() && op.m_Group.empty()) // case where neither have global exponent nor group
	{
		if (op.m_Value != 1 && !power.m_Variables.empty()) // case where exponent has variables
		{
			result.m_Value = 1; // sets new magnitude to 1
			strbuff << op.m_Value;
			result.m_Variables.push_back(strbuff.str()); // pushes old magnitude as variable
			result.m_VarExponent.push_back(new Operand(power)); // pushes exponent of old magnitude
		}
		else
			if (op.m_Value != 1)
				result.m_Value = pow(op.m_Value, power.m_Value);

		list<string>::const_iterator variter = op.m_Variables.begin();
		for (list<Operand*>::const_iterator iter = op.m_VarExponent.begin() // iterates through variables of operand
			; iter != op.m_VarExponent.end(); ++iter, ++variter)
		{
			Operand *newop = NULL;
			if (*variter == COMBINATION_TOKEN) // case where m_VarExponent is a whole expression rather than just exponent
				newop = new Operand(Operand::power(**iter, power));
			else
				newop = new Operand(**iter * power); // calculates new exponent

			result.m_Variables.push_back(*variter); // pushes variable of operand
			result.m_VarExponent.push_back(newop); // pushes new exponent
		}
	}
	else
		if (!power.m_GlobalExp && !op.m_GlobalExp && power.m_Variables.empty() // case where operand has no global exponent and power is a positive integer
			&& power.m_Group.empty() && power.m_Value > 1 && (power.m_Value - floor(power.m_Value) == 0))
		{
			result = op * op;
			for (double exp = power.m_Value - 1; exp > 1; exp--) // recursively unwraps expression
				result = result * op;
		}
		else
		{
			result = op;
			if (op.m_GlobalExp)
			{
				tempPower = power;
				temp = *op.m_GlobalExp;

				if (!power.m_GlobalExp && !power.m_Group.empty()) // wraps power with global exponent of 1 to retain its group structure
					tempPower.m_GlobalExp = new Operand();
				if (!temp.m_GlobalExp && !temp.m_Group.empty()) // wraps operands global exponent with global exponent of 1 to retain its group structure
					temp.m_GlobalExp = new Operand();

				*result.m_GlobalExp = tempPower * temp; // calculates new global exponent
			}
			else
				result.m_GlobalExp = new Operand(power);

			result.cleanUpGlobalExpField();
			if (result.m_GlobalExp)
				result.m_GlobalExp->SimplifyVariableExp();
		}

	result.SimplifyVariableExp();
	result.factorExponent();

	return result;
}

bool Operand::isRealNumber() const {
	if (!m_GlobalExp && m_VarExponent.empty() && m_Variables.empty() && m_Group.empty())
		return true;

	return false;
}

void Operand::factorExponent()
{
	double tempRoot, temp;
	unsigned short root = 2;
	unsigned short maxRoot = 0;
	stringstream strbuff;

	if (m_Group.empty() && !m_GlobalExp && m_Variables.size() == 1 && validateDigit(m_Variables.front())) // can only be performed on operand with single variable which represents a digit with a variable exponent
	{
		temp = strtod(m_Variables.front().c_str(), NULL); // extracts digit
		if (temp - floor(temp) == 0) // checks if digit is an integer
		{
			do
			{
				tempRoot = pow(temp, (double)1 / root);
				if ((tempRoot - floor(tempRoot)) == 0) // saves largest exponent
					maxRoot = root;
				root++;
			} while (tempRoot > 2); // tries all possible exponents

			if (maxRoot != 0)
			{
				temp = pow(temp, (double)1 / maxRoot); // factors out exponent
				m_VarExponent.push_front(new Operand(*m_VarExponent.front() * maxRoot)); // pushes down new exponent
				delete m_VarExponent.back(); // deletes old exponent
				m_VarExponent.pop_back();
				strbuff << temp;
				m_Variables.clear(); // deletes old digit
				m_Variables.push_back(strbuff.str()); // pushes down new digit
			}
		}
	}
}

void Operand::SimplifyVariableExp()
{
	bool tokenSimplified = false;
	Operand temp;
	double temp2;

	list<string>::iterator varIter = m_Variables.begin();
	list<Operand*>::iterator iter = m_VarExponent.begin();
	while (iter != m_VarExponent.end() && !tokenSimplified) // iterates through variables
	{
		if (*varIter == COMBINATION_TOKEN && !(*iter)->m_GlobalExp) // checks if variable is another operand without global exponent, IT MAY HAVE VARIABLES AND GROUPS
		{
			tokenSimplified = true;
			temp = **iter; // saves operand
			varIter = m_Variables.erase(varIter); // deletes this token from variable list
			delete *iter;
			iter = m_VarExponent.erase(iter);
			*this = *this * temp; // multiplies this expression by the operand found amongst variables
		}
		else
			if (validateDigit(*varIter) && validateDigit((*iter)->getId())) // case where variable is a real number with a real number exponent
			{
				tokenSimplified = true;
				temp2 = pow(strtod(varIter->c_str(), NULL), strtod((*iter)->getId().c_str(), NULL)); // calculates extraccted real number
				varIter = m_Variables.erase(varIter);
				delete *iter;
				iter = m_VarExponent.erase(iter);
				*this = *this * temp2; // multiplies this expression by the real number found amongst variables
			}
		if (!tokenSimplified) {
			++iter;
			++varIter;
		}
	}

	if (tokenSimplified) // recursively applies simplification to see if there are more variables to simplify
		SimplifyVariableExp();
}

void Operand::cleanUpGlobalExpField()
{
	if (m_GlobalExp && *m_GlobalExp == 1) // deletes exponent that is equivalent to 1
	{
		delete m_GlobalExp;
		m_GlobalExp = NULL;
	}
	else
		if (m_GlobalExp) // recursively apply this if this exponent has a global exponent of its own
			m_GlobalExp->cleanUpGlobalExpField();

	for (list<Operand*>::iterator iter = m_Group.begin(); iter != m_Group.end(); ++iter) // perform this operation on all members of the group
		(*iter)->cleanUpGlobalExpField();

	for (list<Operand*>::iterator iter = m_VarExponent.begin(); iter != m_VarExponent.end(); ++iter) // perform this operation on exponents of all variables
		(*iter)->cleanUpGlobalExpField();
}

/*
void Operand::fact()
{
if(m_ImaginaryValue == 0 && (m_RealValue - floor(m_RealValue)) == 0 && m_RealValue >= 0)
{
if(m_RealValue == 0)
m_RealValue = 1;
else
for(double i = m_RealValue - 1; i > 1; i--)
m_RealValue = m_RealValue*i;
}
else
throw BAD_FACT_ERROR;
}
*/
void Operand::setBase(const unsigned base)
{
	if (base > 1)
		m_Base = base;
	else
		throw BAD_BASE_ERROR;
}

void Operand::setCustomDesc(const string str) {
	m_CustomDesc = str;
}

string Operand::getId() const
{
	stringstream temp;
	bool globalExpWrapped = false;
	string output = "", comboutput = "", unrelatedgroup = "";
	list<string>::const_iterator variter = m_Variables.begin();

	if (!m_CustomDesc.empty()) {
		return m_CustomDesc;
	}

	if (m_GlobalExp && !m_Group.empty()) // appends open parenthesis if global exponent exists and group non empty
	{
		output += "(";
		globalExpWrapped = true;
	}

	if (m_Variables.empty() || (!m_Variables.empty() && fabs(m_Value) != 1)) // appends magnitude if no variables are present or if it isnt 1 or -1
	{
		temp << m_Value;
		output += temp.str();
	}
	else
		if (m_Value == -1) // appends negative sign if value is -1 and there is at least one variable
			output += "-";

	for (list<Operand*>::const_iterator expiter = m_VarExponent.begin(); expiter != m_VarExponent.end(); ++expiter, ++variter) // iterates through variables
	{
		if (validateDigit(*variter) && ((m_Value != 1 && m_Value != -1) || strtod(variter->c_str(), NULL) < 0)) // appends open parenthesis if magnitude does not equal to 1 or -1 and this "variable" is a real number or "variable" is a negative
			output += "(";

		if (*variter == COMBINATION_TOKEN)
		{
			if (m_GlobalExp) // appends combinatorial operand so its special output if entire expression has a global exponent
				comboutput += (*expiter)->getId();
			else // otherwise appends combinatorial operand to regular output
				output += (*expiter)->getId();
		}
		else
		{
			if ((*expiter)->m_Group.empty() && (*expiter)->m_Variables.empty() && (*expiter)->m_Value == 1) // case where exponent equals to 1
				output += *variter;
			else
			{
				if (validateDigit(*variter) && strtod(variter->c_str(), NULL) < 0) // closing parenthesis for case where "variable" is a negative real number as well as appends exponent and wraps it in parenthesis
					output += *variter + ")^(" + (*expiter)->getId() + ")";
				else // appends exponent and wraps it in parenthesis
					output += *variter + "^(" + (*expiter)->getId() + ")";
			}
		}
		if (validateDigit(*variter) && m_Value != 1 && m_Value != -1) //appends close parenthesis if magnitude does not equal to 1 or -1 and this "variable" is a real number
			output += ")";
	}

	for (list<Operand*>::const_iterator iter = m_Group.begin(); iter != m_Group.end(); ++iter) // iterates through associated group
	{
		if (((*iter)->m_GlobalExp || containsCombToken(*this)) && (*iter)->m_Value < 0) // appends operand to its special output if it has global exponent or expression has combinatorial token and less than 0 magnitude
			unrelatedgroup += (*iter)->getId();
		else
			if ((*iter)->m_GlobalExp || containsCombToken(*this)) // appends operand to its special output if it has global exponent or expression has combinatorial token
				unrelatedgroup += "+" + (*iter)->getId();
			else
				if ((*iter)->m_Value < 0) // appends operand to regular output if it has no global exponent and less than 0 magnitude
					output += (*iter)->getId();
				else // default case for appending this operand
					output += "+" + (*iter)->getId();
	}
	if (globalExpWrapped) // appends closing parenthesis if we are wrapping entire operand with a global exponent
		unrelatedgroup += ")";
	if (m_GlobalExp) // appends global exponent of the entire expression
		unrelatedgroup += "^(" + m_GlobalExp->getId() + ")";

	return output + comboutput + unrelatedgroup;
}

Operator::Operator(ops type)
	: m_Type(type)
{
	switch (type)
	{
	case POW:
		m_Presedence = 4;
		m_leftAssociative = false;
		break;
	case MULT:
	case DIV:
	case MODULO:
		m_Presedence = 3;
		m_leftAssociative = true;
		break;
	case ADD:
	case SUB:
		m_Presedence = 2;
		m_leftAssociative = true;
		break;
	case COMMA:
	case OPENP:
	case CLOSEP:
		m_Presedence = 0;
		m_leftAssociative = false;
		break;
	default:
		throw BAD_TYPE_ERROR;
	}
}

string Operator::getId() const
{
	switch (m_Type)
	{
	case POW:
		return "^";
		break;
	case MULT:
		return "*";
		break;
	case DIV:
		return "/";
		break;
	case ADD:
		return "+";
		break;
	case SUB:
		return "-";
		break;
	case OPENP:
		return "(";
		break;
	case CLOSEP:
		return ")";
		break;
	case MODULO:
		return "%";
		break;
	case COMMA:
		return ",";
		break;
	default:
		throw BAD_TYPE_ERROR;
	}
}

int Operator::getPresedence() const
{
	return m_Presedence;
}

ops Operator::getType() const
{
	return m_Type;
}

bool Operator::isLeftAssociative() const
{
	return m_leftAssociative;
}

Function::Function(string name)
	: m_Name(name)
{
	if (functions.find(name) == functions.end())
		throw UNKNOWN_FUNCTION;
}

string Function::getId() const {
	return m_Name;
}

void Function::call(vector<Operand*> &argList) const {
	map<string, void(*)(vector<Operand*>&)>::iterator it = functions.find(m_Name);

	if (it != functions.end()) {
		it->second(argList);
	}
	else {
		throw UNKNOWN_FUNCTION;
	}
}

void Function::addFunction(string name, void(*fncAddr)(vector<Operand*>&)) {
	functions[name] = fncAddr;
}

void Function::removeFunction(string name) {
	functions.erase(name);
}

bool Function::functionExists(string name) {
	if (functions.find(name) != functions.end())
		return true;
	return false;
}

size_t Function::functionCount() {
	return functions.size();
}

void rectPrismArea_f(std::vector<Operand *> &argList) { // calculates area of rectangular prism
	if (argList.size() != 3)
		throw BAD_ARG_SIZE;

	*argList[0] = 2 * (*argList[0] * (*argList[1]) + *argList[0] * (*argList[2]) + *argList[1] * (*argList[2]));
}

void log10_f(vector<Operand*> &argList) { // calculates log of base 10
	Operand *temp;
	double val;

	if (argList.size() != 1)
		throw BAD_ARG_SIZE;

	temp = argList[0];
	val = strtod(temp->getId().c_str(), NULL);
	if (!temp->isRealNumber() || val < 0.0)
		throw LOG_ERROR;

	*temp = log10(val);
}

void ln_f(vector<Operand*> &argList) { // calculates log of base e (natural log)
	Operand *temp;
	double val;

	if (argList.size() != 1)
		throw BAD_ARG_SIZE;

	temp = argList[0];
	val = strtod(temp->getId().c_str(), NULL);
	if (!temp->isRealNumber() || val < 0.0)
		throw LOG_ERROR;

	*temp = log(val);
}
