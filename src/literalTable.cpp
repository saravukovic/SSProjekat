#include "../inc/literalTable.h"

Literal::Literal(string literalName, unsigned int literalAdress, int literalValue)
{
	this->literalName = literalName;
	this->literalAdress = literalAdress;
	this->literalValue = literalValue;
}

Literal::~Literal()
{
}

int Literal::getLiteralValue()
{
	return literalValue;
}

void Literal::setLiteralAdress(unsigned int adr)
{
	this->literalAdress = adr;
}

unsigned int Literal::getLiteralAdress()
{
	return this->literalAdress;
}

string Literal::getLiteralName()
{
	return this->literalName;
}
