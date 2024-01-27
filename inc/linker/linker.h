#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <iomanip>
#include "../symbolTable.h"
#include "../sectionTable.h"
#include "../literalTable.h"
#include "../relocationTable.h"
#include "../syntaxCheck.h"

using namespace std;

class Linker
{
public:
	Linker(list<string> input, string output, list<string> placement);
	~Linker();
	void link();

private:

	void parseBinaryInput(string inputFile);
	void createSymbolTable();
	void createSectionTable();
	void resolveAllSymbols();
	void createHEX();
	void createBinary();


	map<string, map<string, SymbolDefinition>> symbolTables;
	map<string, map<string, SectionDefinition*>> sectionTables;
	map<string, SymbolDefinition> symbolTable;
	map<string, SectionDefinition*> sectionTable;
	map<string, unsigned int> placements;
	map<string, SymbolDefinition> undefinedSymbols;
	unsigned int pc;
	list<string> inputFilesLinker, inputFileNameArrLinker, placement;
	string outputFileLinker, outputFileNameLinker;
	string currSection;
	list<SectionDefinition*> sortedListOfSections;
};