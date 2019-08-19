#pragma once

#include <string>
#include <list>
#include <map>
#include <vector>

const unsigned DEFAULT_BASE = 10; // default base of the operand
const double DEFAULT_VALUE = 1; // default value of the operand
const double DEFAULT_EXPONENT = 1; // default exponent value of the operand
const std::string COMPLEX_TOKEN = "i"; // denotes complex number
const std::string COMBINATION_TOKEN = "<>"; // denotes variable which is another operand with non NULL m_GlobalExp

const std::string LOG_FNC = "log";
const std::string LN_FNC = "ln";
const std::string RECT_PRISM_AREA = "rectprismarea";
const std::string ATLAS_MAXSHIP_DMG = "atlasmaxshipdmg";

enum ops { POW, MULT, DIV, ADD, SUB, OPENP, CLOSEP, MODULO, COMMA }; // basic operators

class Atom // base class
{
public:
	virtual std::string getId() const = 0;
	virtual ~Atom();
};

class Operand : public Atom
{
public:
	/* PRE:
	* POST: creates operand with magnitude of DEFAULT_VALUE
	* USES:
	* THROWS:
	*/
	Operand();

	/* PRE:
	* POST: creates operand with desired magnitude and base
	* USES:
	* THROWS: BAD_BASE_ERROR
	*/
	Operand(double value, unsigned base = DEFAULT_BASE);

	/* PRE:
	* POST: creates operand with desired variable and base
	* USES:
	* THROWS: BAD_BASE_ERROR, BAD_VAR_NAME
	*/
	Operand(std::string variable, unsigned base = DEFAULT_BASE);

	/* PRE:
	* POST: object is destroyed and all memory released
	* USES: clearOpList()
	* THROWS:
	*/
	~Operand();

	/* PRE:
	* POST: creates a copy of other operand
	* USES: copyOpList()
	* THROWS: BAD_VAR_SIZE
	*/
	Operand(const Operand &other);

	/* PRE:
	* POST: sets this operand to desired magnitude, destroying all prior structure
	* USES: clearOpList()
	* THROWS:
	*/
	Operand& operator=(const double value);

	/* PRE: other is a proper object
	* POST: object has same structure as other
	* USES: clearOpList(), copyOpList()
	* THROWS: BAD_VAR_SIZE
	*/
	Operand& operator=(const Operand &other);

	/* PRE: other is a proper object
	* POST: true if both objects are structurally equivalent
	* USES: sameGroup(), sameGlobalExponent()
	* THROWS:
	*/
	bool operator==(const Operand &other) const;

	/* PRE:
	* POST: true if this expression is equivalent to desired double
	* USES:
	* THROWS:
	*/
	bool operator==(const double value) const;

	/* PRE: other is a proper object
	* POST: true if both objects are not structurally equivalent
	* USES:
	* THROWS:
	*/
	bool operator!=(const Operand &other) const;

	/* PRE:
	* POST: true if this expression is not equivalent to desired double
	* USES:
	* THROWS:
	*/
	bool operator!=(const double value) const;

	/* PRE: op1 and op2 are proper objects
	* POST: an object that is a sum of op1 and op2
	* USES: clearGroup(), canAdd(), simpZero(), self recursive
	* THROWS: BAD_GROUP, BASE_ERROR
	*/
	friend Operand operator+(const Operand &op1, const Operand &op2);

	/* PRE: op1 and op2 are proper objects
	* POST: an object op1%op2 is returned
	* USES:
	* THROWS:
	*/
	friend Operand operator%(const Operand &op1, const Operand &op2);

	/* PRE: left and right are proper objects
	* POST: an object that is a difference of left and right
	* USES: isProperInt()
	* THROWS: MODULO_ERROR
	*/
	friend Operand operator-(const Operand &left, const Operand &right);

	/* PRE: op is a proper object
	* POST: negation of original expression
	* USES:
	* THROWS:
	*/
	friend Operand operator-(const Operand &op);

	/* PRE: op1 and op2 are proper objects
	* POST: an object that is a product of op1 and op2
	* USES: clearGroup(), combineOps(), clearGlobalExpGroup(), combineGlobalExpOps(), self recursive
	* THROWS: BASE_ERROR
	*/
	friend Operand operator*(const Operand &op1, const Operand &op2);

	/* PRE: left and right are proper objects
	* POST: an object that is a quotient of left and right
	* USES: power()
	* THROWS: DIV_ZERO_ERROR
	*/
	friend Operand operator/(const Operand &left, const Operand &right);

	/* PRE: op and power are proper objects
	* POST: an object that is a result of taking op to an exponent of power
	* USES: cleanUpGlobalExpField(), SimplifyCombTokens(), factorExponent(), self recursive
	* THROWS: BASE_ERROR
	*/
	static Operand power(const Operand &op, const Operand &power);

	/* PRE:
	* POST: this objects base is now set to a new value
	* USES:
	* THROWS: BAD_BASE_ERROR
	*/
	void setBase(const unsigned base);

	/* PRE:
	* POST: sets custom description string for this operand
	* USES:
	* THROWS:
	*/
	void setCustomDesc(const std::string str);

	/* PRE:
	* POST: a string that describes this expression using infix notation
	* USES: self recursive
	* THROWS:
	*/
	virtual std::string getId() const;

	/* PRE:
	* POST: returns true if input string is a valid real number
	* USES:
	* THROWS:
	*/
	static bool validateDigit(const std::string &digit);

	/* PRE:
	* POST: returns true if this operand is a real number
	* USES:
	* THROWS:
	*/
	bool isRealNumber() const;

private:
	/* PRE:
	* POST: list is now empty and all its elements are deleted
	* USES:
	* THROWS:
	*/
	static void clearOpList(std::list<Operand*> &list);

	/* PRE:
	* POST: all elements from source list are now copied to dest list
	* USES:
	* THROWS:
	*/
	static void copyOpList(std::list<Operand*> &dest, const std::list<Operand*> &source);

	/* PRE: op is a proper object
	* POST: all operands with magnitude of zero have been removed
	* USES:
	* THROWS:
	*/
	static void simpZero(Operand &op);

	/* PRE: left and right are objects whos m_GlobalExp are NULL
	* POST: product of left and right object
	* USES: clearGroup(), validateDigit(), adjustComplex()
	* THROWS:
	*/
	static Operand combineOps(const Operand &left, const Operand &right);

	/* PRE: op1 and op2 are objects where at least one has non NULL m_GlobalExp
	* POST: product of op1 and op2
	* USES: getCombExprs(), clearGlobalExpCombGroup(), combineGlobalExpOps2()
	* THROWS:
	*/
	static Operand combineGlobalExpOps(const Operand &op1, const Operand &op2);

	/* PRE: extension of combineGlobalExpOps(), only to be used within its scope
	* POST:  product of op1 and op2
	* USES:  sameVariables(), sameGroup()
	* THROWS:
	*/
	static Operand combineGlobalExpOps2(const Operand &left, const Operand &right);

	/* PRE: op is a proper object
	* POST: list populated with references to operands with COMBINATION_TOKEN
	* USES: sameGroup()
	* THROWS:
	*/
	static std::list<Operand*> getCombExprs(const Operand &op);

	/* PRE: op1 and op2 are proper objects
	* POST: true if both objects are indentical except for their m_GlobalExp, m_Value, and m_Base
	* USES: clearGlobalExpCombGroup(), containsCombToken(), sameGroupTwo(), sameVariables(), getCombExprs()
	* THROWS:
	*/
	static bool sameGroup(const Operand &op1, const Operand &op2);

	/* PRE: op1 and op2 do not have any COMBINATION_TOKEN and both have non empty m_Group, this is an extension of sameGroup()
	* POST: true if both objects are identical sums
	* USES: clearGroup()
	* THROWS:
	*/
	static bool sameGroupTwo(const Operand &op1, const Operand &op2);

	/* PRE: exp1 and exp2 are either NULL pointers or to proper objects
	* POST: true if both objects are identical
	* USES:
	* THROWS: BAD_EXP_COMP
	*/
	static bool sameGlobalExponent(const Operand *const exp1, const Operand *const exp2);

	/* PRE: op is a proper object
	* POST: true if COMBINATION_TOKEN is present amongst m_Variables
	* USES:
	* THROWS:
	*/
	static bool containsCombToken(const Operand &op);

	/* PRE: op1 and op2 are proper objects
	* POST: true if m_Variables lists from both objects are identical
	* USES:
	* THROWS:
	*/
	static bool sameVariables(const Operand &op1, const Operand &op2);

	/* PRE: op1 and op2 are proper objects
	* POST: true if two objects can be added
	* USES: sameGroup(), sameGlobalExponent()
	* THROWS:
	*/
	static bool canAdd(const Operand &op1, const Operand &op2);

	/* PRE:
	* POST: true if this is an integer
	* USES:
	* THROWS:
	*/
	bool isProperInt() const;

	/* PRE:
	* POST: if object has a single variable who's form is x^y where x is a real number then it is re-factored
	*       into b^(ay) where a is largest root of x, otherwise does nothing
	* USES:
	* THROWS:
	*/
	void factorExponent();

	/* PRE:
	* POST: all objects with m_GlobalExp are removed from this objects m_Group
	* USES:
	* THROWS:
	*/
	void clearGlobalExpGroup();

	/* PRE:
	* POST: all variables with COMBINATION_TOKEN are removed from this objects m_Variables, along with their m_VarExponent
	* USES:
	* THROWS:
	*/
	void clearGlobalExpCombGroup();

	/* PRE:
	* POST: removes all objects from this objects m_Group
	* USES:
	* THROWS:
	*/
	void clearGroup();

	/* PRE:
	* POST: m_GlobalExp fields whos value is 1 have been removed
	* USES: self recursive
	* THROWS:
	*/
	void cleanUpGlobalExpField();

	/* PRE:
	* POST: all variables whos exponent or m_GlobalExp is 1 have been simplified
	* USES: self recursive, validateDigit()
	* THROWS:
	*/
	void SimplifyVariableExp();

	/* PRE:
	* POST: complex number has been simplified
	* USES:
	* THROWS:
	*/
	void adjustComplex(std::list<Operand*> &varexp, std::list<Operand*>::iterator &varexppos
		, std::list<std::string> &variables, std::list<std::string>::iterator &varpos);

	/* PRE:
	* POST: true if m_Variables contains only COMBINATION_TOKEN
	* USES:
	* THROWS:
	*/
	bool combExprOnly() const;

	/* PRE:
	* POST: true if m_Variables contains at least case of COMBINATION_TOKEN
	* USES:
	* THROWS:
	*/
	bool containsCombToken() const;

	/* PRE:
	* POST: true if no objects in m_Group have m_GlobalExp
	* USES:
	* THROWS:
	*/
	bool singleGroupExpr() const;

	double m_Value; // real number value of the expression (magnitude)
	Operand *m_GlobalExp; // global exponent, that extends across all groups and variables
	unsigned m_Base; // base of the expression
	std::string m_CustomDesc; // custom value of expression
	std::list<std::string> m_Variables; // list of variables who belong to this expresion
	std::list<Operand*> m_VarExponent; // list of exponents who belong to variables of this expression (ex: exponent at pos2 belongs to variable at pos2)
	std::list<Operand*> m_Group; // list of additive expressions

	static bool m_Combined; // control variable used in taking product of objects when at least one has non NULL m_GlobalExp, this makes any multithreading impossible and must go
};

class Operator : public Atom
{
public:
	/* PRE:
	* POST: specified operator is created
	* USES:
	* THROWS: BAD_TYPE_ERROR
	*/
	Operator(const ops type);

	/* PRE:
	* POST: string that describes this operator
	* USES:
	* THROWS: BAD_TYPE_ERROR
	*/
	virtual std::string getId() const;

	/* PRE:
	* POST: presedence value is returned
	* USES:
	* THROWS:
	*/
	int getPresedence() const;

	/* PRE:
	* POST: type value is returned
	* USES:
	* THROWS:
	*/
	ops getType() const;

	/* PRE:
	* POST: true if operator is left associative
	* USES:
	* THROWS:
	*/
	bool isLeftAssociative() const;
private:
	const ops m_Type; // type of the operator
	int m_Presedence; // presedence value of the operator
	bool m_leftAssociative; // whether operator is left or right associative
};

class Function : public Atom {
public:
	/* PRE: funcions map is populated
	* POST: function object with appropriate name and argument size is created
	* USES:
	* THROWS: UNKNOWN_FUNCTION
	*/
	Function(std::string name);

	/* PRE:
	* POST: functions name is returned
	* USES:
	* THROWS:
	*/
	virtual std::string getId() const;

	/* PRE:
	* POST: function is called with provided argument list and result is stored at index 0
	* USES:
	* THROWS: UNKNOWN_FUNCTION
	*/
	void call(std::vector<Operand*> &argList) const;

	/* PRE:
	* POST: function name and reference added to the known function map, existing function is overwritten
	* USES:
	* THROWS:
	*/
	static void addFunction(std::string name, void(*fncAddr)(std::vector<Operand*>&));

	/* PRE:
	* POST: function reference with given name is removed from the known function map
	* USES:
	* THROWS:
	*/
	static void removeFunction(std::string name);

	/* PRE:
	* POST: returns true if given function name has a known function reference
	* USES:
	* THROWS:
	*/
	static bool functionExists(std::string name);

	/* PRE:
	* POST: returns number of known functions
	* USES:
	* THROWS:
	*/
	static size_t functionCount();

private:
	static std::map<std::string, void(*)(std::vector<Operand*>&)> functions; // maps function name to its function pointer
	std::string m_Name; // function name
};

/*list of pre defined functions*/
void log10_f(std::vector<Operand*> &argList);
void ln_f(std::vector<Operand*> &argList);
void rectPrismArea_f(std::vector<Operand*> &argList);
void atlasMaxShipDamage_f(std::vector<Operand*> &argList);
