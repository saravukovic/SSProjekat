#include "../inc/symbolTable.h"

int SymbolDefinition::globalSymId = 0;

SymbolDefinition::SymbolDefinition(string symName, string secName, int secNum, SymbolType type, SymbolBinding bind, unsigned int adress, bool isExtern)
{
	this->symName = symName;
	this->symID = globalSymId++;
	this->secName = secName; //mozda je ovo bespotrebno
	this->secNum = secNum; //ili ovo - ugl pretpostavljam nesto od ta dva
	this->symType = type;
	this->symBind = bind;
	this->adress = adress;
	this->isExtern = isExtern;
	if(this->adress != 0 || secName != "") this->defined = true;
	else this->defined = false;
}

SymbolDefinition::~SymbolDefinition()
{
}

bool SymbolDefinition::isSymbolDefined()
{
	return defined;
}

bool SymbolDefinition::isSymbolExtern()
{
	return isExtern;
}

void SymbolDefinition::defineSymbol(unsigned int adress, string secName, int secID)
{
	defined = true;
	this->adress = adress;
	this->secName = secName;
	this->secNum = secID;
}

void SymbolDefinition::setSymAsGlobal()
{
	symBind = GLOBAL;
}

SymbolBinding SymbolDefinition::getSymbolBinding()
{
	return symBind;
}

unsigned int SymbolDefinition::getSymAdress()
{
	return this->adress;
}

int SymbolDefinition::getSymID()
{
	return this->symID;
}

void SymbolDefinition::setSymID(int id)
{
	this->symID = id;
}

int SymbolDefinition::getSecNum()
{
	return this->secNum;
}

SymbolType SymbolDefinition::getSymType()
{
	return this->symType;
}

string SymbolDefinition::getSymbolsSection()
{
	return this->secName;
}

void SymbolDefinition::resetSymID()
{
	globalSymId = 0;
}
