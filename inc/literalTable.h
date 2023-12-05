#pragma once
#include <string>

using namespace std;

class Literal
{
public:

	Literal(string literalName = "", unsigned int literalAdress = 0, int literalValue = 0);
	~Literal();

	int getLiteralValue();
	void setLiteralAdress(unsigned int adr);
	unsigned int getLiteralAdress();
	string getLiteralName();

private:
	unsigned int literalAdress = 0;
	unsigned int literalValue = 0;
	string literalName;
};
