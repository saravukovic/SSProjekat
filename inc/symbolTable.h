#pragma once

#include <string>

using namespace std;

enum SymbolType
{
	NOTYPE,
	SECTION
};

enum SymbolBinding
{
	LOCAL,
	GLOBAL
};

class SymbolDefinition
{
public:

	SymbolDefinition(string symName, string secName, int secNum, SymbolType type, SymbolBinding bind, unsigned int adress, bool isExtern);
	~SymbolDefinition();

	bool isSymbolDefined();
	bool isSymbolExtern();
	void defineSymbol(unsigned int adress, string secName, int secID);
	void setSymAsGlobal();
	SymbolBinding getSymbolBinding();
	unsigned int getSymAdress();
	int getSymID();
	void setSymID(int id);
	int getSecNum();
	SymbolType getSymType();
	string getSymbolsSection();

	static void resetSymID();


private:
	static int globalSymId;
	string symName, secName;
	SymbolType symType;
	SymbolBinding symBind;
	int symID, secNum;
	unsigned int adress;
	bool defined;
	bool isExtern;
};
