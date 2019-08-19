// Win32Project1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "calculator.h"
#include "ipcmodule.h"
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <Windows.h>
#include <process.h>
#include <regex>
#include <vector>
#include <map>

using namespace std;

const bool TESTS_TO_RUN = true;

int main(int argc, char* argv[])
{
	CalcIPC comms;
	Operand op1("A");
	Operand op2("B");
	Operand op3("c");
	Operand op4("d");
	Operand op5("F");
	Operand op7(3);
	Operand op8(3);
	Operand op9;
	Operand op10;
	list<Operand> testOps;
	string input[] = { " a+a-b+b+c+c-b-c = ", " (a+b)^b+(b+c)^c = ",  " (a+b)^b*(b+c)^c = ", " (b+c)^c*(a+b)^b = ",
		" ((a+b)^b*(b+c)^c + 1)+(b+c)^c*(a+b)^b = ", " a*a/b*b/b*c/c = ", " (a+b)^c+(b+a)^c = " , " (a+b)^(b)(c+d)^(d)+(d+c)^(d)(b+a)^(b) = " , " 3^a+3^b = ", " (a+bc)^2/a = ",
		" 2*3^a*2*3^b = ", " (2*3^a * 2*3^b)^2 = ", " 3^a*2a = ", " (3^a)^2 = ", " (3^a + 3^b)^2/9^a = ",
		" a * 1/a = ", " 4(a+b)^c + 5(a+b)^c = ", " 5^a * 7^a = ", " (a+b)^c * (a+b)^d = ", " (a+b)^c/(a+b)^c = ",
		" ((a+b)^c + (a+b)^d)/(a+b)^c = ", " (a+b)^c * (d+f)^c = ", " ((a+b)^c + b)^d + ((a+b)^c + b)^d = ",
		" ((a+b)^c + b)^d * ((a+b)^c + b)^d = ", " a((a+b)^c + b)^d + a((a+b)^c + b)^d = ", " a((a+b)^c + b)^d * a((a+b)^c + c)^c = ",
		" a*((a+b)^c + b)^d * b((a+b)^c + c)^c = ", " a((a+b)^c + b)^d * b((a+b)^c + c)^d = ", " a((3^a+3^b)^c + b)^d * b((3^a+3^b)^c + c)^d = ",
		" a((3^a+b)^c + b)^d * b((3^a+b)^c + c)^d = ", " a((3^a+b)^c + b)^d * b((b+3^a)^c + c)^d = ", " a((3^a+b)^c + b)^d + a((b+3^a)^c + b)^d = ",
		" a((3^a+b)^c + b)^d + a((b+3^a)^c + b)^d = ", " a((3^a+b)^c + b)^d + b + a((3^a+b)^c + b)^d = ", " (a+b)^c + b + (b+c)^d + a + (a+b)^c = ",
		" 3^(a+b) * 3^(a-b) = ", " 3^(a+b) * 3^(b-a) = ", " 3^(5a+2b-c) * 3^(-2c-5a+3b) = ", " a^c * b^c = ", " (4^(3a))^(-a) = ",  " (4^(3a))^(1/a) = ",
		" (4^(5*7^(a+b)))^(1/(7^(a+b))) = ", " (4^(7^a))^(1/7^a) = ", " 1/a = ", " (a^(b+c))^(1/(b+c)) = ", " (a^b)^(1/b) = ", " 1/(a+b+c) = ",
		" (a+b+c)/(b+c+a) = ", " (a+b+c)/(b+c+d) = ", " a^(a+b)*b^(a+d) = ", " a^(a+b)*a^(a+b)*(-1) = ", " 12^a * (-1)3^a = ", " 2^a * (-8)^a = ",
		" (a+1)^b*(b+2)^a*(c+3)^d*(d+4)^c + (c+3)^d*(b+2)^a*(d+4)^c*(a+1)^b = ", " ((b+(a+b)^(-a))^(1/b))^b * (b+a)^a = ",
		" (((b+(a+b)^(a^(-1)))^(b^(-1)))^b-b)^a-b = ", " 0+a = "};

	string output[] = { "2a+c-b", "(a+b)^(b)+(b+c)^(c)", "(a+b)^(b)(b+c)^(c)", "(b+c)^(c)(a+b)^(b)", "2(b+c)^(c)(a+b)^(b)+1", "a^(2)b^(-1)", "2(a+b)^(c)", "2(c+d)^(d)(a+b)^(b)",
		"3^(a)+3^(b)", "a+2bc+b^(2)c^(2)a^(-1)", "4(3^(a+b))", "16(3^(2a+2b))", "2(3^(a))a", "3^(2a)", "1+2(3^(b-a))+3^(2b-2a)",
		"1", "9(a+b)^(c)", "35^(a)", "(a+b)^(c+d)", "1", "1+(a+b)^(d-c)", "(ad+bd+af+bf)^(c)", "2((a+b)^(c)+b)^(d)", "((a+b)^(c)+b)^(2d)", "2a((a+b)^(c)+b)^(d)",
		"a^(2)((a+b)^(c)+b)^(d)((a+b)^(c)+c)^(c)", "ab((a+b)^(c)+b)^(d)((a+b)^(c)+c)^(c)", "ab((a+b)^(2c)+b(a+b)^(c)+(a+b)^(c)c+bc)^(d)",
		"ab((3^(a)+3^(b))^(2c)+b(3^(a)+3^(b))^(c)+(3^(a)+3^(b))^(c)c+bc)^(d)", "ab((3^(a)+b)^(2c)+b(3^(a)+b)^(c)+(3^(a)+b)^(c)c+bc)^(d)",
		"ab((b+3^(a))^(2c)+b(b+3^(a))^(c)+(3^(a)+b)^(c)c+bc)^(d)", "2a((3^(a)+b)^(c)+b)^(d)", "2a((3^(a)+b)^(c)+b)^(d)", "2a((3^(a)+b)^(c)+b)^(d)+b",
		"2(a+b)^(c)+b+(b+c)^(d)+a", "3^(2a)", "3^(2b)", "3^(5b-3c)", "a^(c)b^(c)", "2^(-6a^(2))", "64", "1024", "4", "a^(-1)", "a", "a", "(a+b+c)^(-1)", "1",
		"a(b+c+d)^(-1)+b(b+c+d)^(-1)+c(b+c+d)^(-1)", "a^(a+b)b^(a+d)", "-a^(2a+2b)", "-6^(2a)", "(-2)^(4a)", "2(d+4)^(c)(c+3)^(d)(b+2)^(a)(a+1)^(b)",
		"b(b+a)^(a)+1", "a", "a"};
	unsigned short arrayIter = 0;
	unsigned short passCount = 0;
	string temp;
	calc* calculator = calc::access();

	testOps.push_back(op1 + op1 - op2 + op2 + op3 + op3 - op2 - op3); // 1
	testOps.push_back(Operand::power(op1 + op2, op2) + Operand::power(op2 + op3, op3)); //2
	testOps.push_back(Operand::power(op1 + op2, op2) * Operand::power(op2 + op3, op3)); //3
	testOps.push_back(Operand::power(op2 + op3, op3) * Operand::power(op1 + op2, op2)); //4
	testOps.push_back(Operand::power(op1 + op2, op2) * Operand::power(op2 + op3, op3) + 1 + Operand::power(op2 + op3, op3) * Operand::power(op1 + op2, op2)); //5
	testOps.push_back(op1 * op1 / op2 * op2 / op2 * op3 / op3); // 6
	testOps.push_back(Operand::power(op1 + op2, op3) + Operand::power(op2 + op1, op3)); // 7
	testOps.push_back(Operand::power(op1 + op2, op2) * Operand::power(op3 + op4, op4) + Operand::power(op4 + op3, op4) * Operand::power(op2 + op1, op2)); //8
	testOps.push_back(Operand::power(op7, op1) + Operand::power(op8, op2)); //9
	testOps.push_back(Operand::power(op1 + op2 * op3, 2) / op1); // 10
	testOps.push_back((Operand::power(op7, op1) + Operand::power(op7, op1)) * (Operand::power(op8, op2) + Operand::power(op8, op2))); // 11
	testOps.push_back(Operand::power((Operand::power(op7, op1) + Operand::power(op7, op1)) * (Operand::power(op8, op2) + Operand::power(op8, op2)), Operand(2))); //12
	testOps.push_back(Operand::power(op7, op1) * (op1 * 2)); // 13
	testOps.push_back(Operand::power(Operand::power(op7, op1), 2)); // 14
	testOps.push_back(Operand::power((Operand::power(op7, op1) + Operand::power(op8, op2)), 2) / Operand::power(9, op1)); //15
	testOps.push_back(op1 * Operand::power(op1, -1)); //16
	testOps.push_back(4 * Operand::power(op1 + op2, op3) + 5 * Operand::power(op1 + op2, op3)); // 17
	testOps.push_back(Operand::power(5, op1) * Operand::power(7, op1)); // 18
	testOps.push_back(Operand::power(op1 + op2, op3) * Operand::power(op1 + op2, op4)); // 19
	testOps.push_back((Operand::power(op1 + op2, op3)) / Operand::power(op1 + op2, op3)); // 20
	testOps.push_back((Operand::power(op1 + op2, op3) + Operand::power(op1 + op2, op4)) / Operand::power(op1 + op2, op3)); //21
	testOps.push_back(Operand::power(op1 + op2, op3) * Operand::power(op4 + op5, op3)); // 22
	op9 = Operand::power(op1 + op2, op3) + op2;
	op10 = Operand::power(op9, op4);
	testOps.push_back(op10 + op10); // 23
	testOps.push_back(op10 * op10); // 24
	testOps.push_back(op1*op10 + op1*op10); // 25
	testOps.push_back(op1*op10 * op1 * Operand::power(Operand::power(op1 + op2, op3) + op3, op3)); // 26
	testOps.push_back(op1*op10 * op2 * Operand::power(Operand::power(op1 + op2, op3) + op3, op3)); // 27
	testOps.push_back(op1*op10 * op2 * Operand::power(Operand::power(op1 + op2, op3) + op3, op4)); // 28
	op7 = Operand::power(op7, op1);
	op8 = Operand::power(op8, op2);
	op9 = Operand::power(op7 + op8, op3) + op2;
	op10 = Operand::power(op9, op4);
	testOps.push_back(op1*op10 * op2 * Operand::power(Operand::power(op7 + op8, op3) + op3, op4)); // 29
	op9 = Operand::power(op7 + op2, op3) + op2;
	op10 = Operand::power(op9, op4);
	testOps.push_back(op1*op10 * op2 * Operand::power(Operand::power(op7 + op2, op3) + op3, op4)); // 30
	testOps.push_back(op1*op10 * op2 * Operand::power(Operand::power(op2 + op7, op3) + op3, op4)); // 31
	testOps.push_back(op1*op10 + op1 * op10); // 32
	testOps.push_back(op1*op10 + op1 * Operand::power(Operand::power(op2 + op7, op3) + op2, op4)); // 33
	testOps.push_back(op1*op10 + op2 + op1 * op10); // 34
	testOps.push_back(Operand::power(op1 + op2, op3) + op2 + Operand::power(op2 + op3, op4) + op1 + Operand::power(op1 + op2, op3)); // 35
	op7 = Operand::power(3, op1 + op2);
	op8 = Operand::power(3, op1 - op2);
	testOps.push_back(op7 * op8); // 36
	op8 = Operand::power(3, op2 - op1);
	testOps.push_back(op7 * op8); // 37
	op7 = Operand::power(3, 5 * op1 + 2 * op2 - op3);
	op8 = Operand::power(3, -2 * op3 - 5 * op1 + 3 * op2);
	testOps.push_back(op7 * op8); // 38
	testOps.push_back(Operand::power(op1, op3) * Operand::power(op2, op3)); // 39
	op7 = Operand::power(4, 3 * op1);
	testOps.push_back(Operand::power(op7, -1 * op1)); // 40
	testOps.push_back(Operand::power(op7, Operand::power(op1, -1))); // 41
	op7 = Operand::power(4, 5 * Operand::power(7, op1 + op2));
	testOps.push_back(Operand::power(op7, 1 / Operand::power(7, op1 + op2))); // 42
	op7 = Operand::power(4, Operand::power(7, op1));
	testOps.push_back(Operand::power(op7, 1 / Operand::power(7, op1))); // 43
	testOps.push_back(1 / op1); // 44
	op7 = Operand::power(op1, op2 + op3);
	testOps.push_back(Operand::power(op7, 1 / (op2 + op3))); // 45
	op7 = Operand::power(op1, op2);
	testOps.push_back(Operand::power(op7, 1 / op2)); // 46
	testOps.push_back(Operand::power(op1 + op2 + op3, -1)); // 47
	testOps.push_back((op1 + op2 + op3) / (op2 + op3 + op1)); // 48
	testOps.push_back((op1 + op2 + op3) / (op2 + op3 + op4)); // 49
	testOps.push_back(Operand::power(op1, op1 + op2) * Operand::power(op2, op1 + op4)); // 50
	testOps.push_back(Operand::power(op1, op1 + op2) * Operand::power(op1, op1 + op2) * (-1)); // 51
	testOps.push_back(Operand::power(12, op1) * -1 * Operand::power(3, op1)); // 52
	testOps.push_back(Operand::power(2, op1) * Operand::power(-8, op1)); // 53
	op7 = Operand::power(op3 + 3, op4) * Operand::power(op4 + 4, op3);
	op7 = Operand::power(op2 + 2, op1) * op7;
	op7 = Operand::power(op1 + 1, op2) * op7;
	op8 = Operand::power(op4 + 4, op3) * Operand::power(op1 + 1, op2);
	op8 = op8 * Operand::power(op2 + 2, op1);
	op8 = op8 * Operand::power(op3 + 3, op4);
	testOps.push_back(op7 + op8); // 54
	op7 = Operand::power(op1 + op2, -1 * op1);
	op7 = Operand::power(op2 + op7, 1 / op2);
	testOps.push_back(Operand::power(op7, op2) * Operand::power(op2 + op1, op1)); // 55
	op7 = Operand::power(op1 + op2, 1 / op1);
	op7 = Operand::power(op2 + op7, 1 / op2);
	op7 = Operand::power(op7, op2) - op2;
	testOps.push_back(Operand::power(op7, op1) - op2); // 56
	testOps.push_back(Operand(0) + op1); // 57

	if (TESTS_TO_RUN)
	{
		for (list<Operand>::iterator iter = testOps.begin(); iter != testOps.end(); iter++, arrayIter++)
		{
			cout << arrayIter + 1 << ")";
			if (iter->getId() == output[arrayIter])
			{
				cout << "   PASS    " + input[arrayIter] + iter->getId() << endl;
				passCount++;
			}
			else
				cout << "   FAIL    " + input[arrayIter] + iter->getId() + "    EXPECTED: " + output[arrayIter] << endl;
		}
		cout << "\nPASSED: " << passCount << "/" << testOps.size() << " TESTS\n\n\n";
		passCount = 0;

		for (arrayIter = 0; arrayIter < sizeof(input) / sizeof(string); arrayIter++) {
			cout << arrayIter + 1 << ")";
			temp = calculator->solve(input[arrayIter].substr(1, input[arrayIter].size()-3));
			if (temp == output[arrayIter]) {
				cout << "   PASS    " + input[arrayIter] + temp << endl;
				passCount++;
			}
			else {
				cout << "   FAIL    " + input[arrayIter] + temp + "    EXPECTED: " + output[arrayIter] << endl;
			}
		}
		cout << "\nPASSED: " << passCount << "/" << testOps.size() << " TESTS" << endl;
	}

	comms.exec();
	return EXIT_SUCCESS;
}

