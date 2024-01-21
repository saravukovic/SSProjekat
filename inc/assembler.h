#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <iomanip>
#include "symbolTable.h"
#include "sectionTable.h"
#include "literalTable.h"
#include "relocationTable.h"
#include "syntaxCheck.h"

#define MAXLITVAL	2047

using namespace std;

class Assembler
{
public:
	Assembler(string input, string output);
	~Assembler();

	void assembler();


private:
	void firstPass();
	void secondPass();

	void labelInFirstPass(string* line);
	void instructionInFirstPass(string* instr);
	void directiveInFirstPass(string* instr);

	void checkAndAddToLiteralTable(string operand);
	void checkOperandAndAddToLiteralTable(string operand);
	void checkDirectiveListOfSymbols(string listSym, TypeOfDirective dir);
	void secAndEndDirective(string secName, TypeOfDirective dir);

	void labelInSecondPass(string* line);
	void instructionInSecondPass(string* instr);
	void directiveInSecondPass(string* instr);
	void getSymOrLitAdrSecPass(string operand, char* adress);
	void loadInstrHandler(string operand, string reg);
	void storeInstrHandler(string operand, string reg);

	void secAndEndDirectiveSecondPass(string secName, TypeOfDirective dir);

	void createELF();
	void createBinary();

	char regToChar(string reg);

	map<string, SymbolDefinition> symbolTable;
	map<string, SectionDefinition*> sectionTable;
	unsigned int pc, numOfLine, currSectionNum;
	string inputFile, outputFile;
	string inputFileName, outputFileName;
	string currSection;
	SyntaxCheck parser;
	bool ldLitMem, assembling, symbolDefinedInSection;
	list<string> formattedInputFile;

};
