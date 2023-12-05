#pragma once
#include <string>
#include <list>
#include "relocationTable.h"
#include "literalTable.h"

using namespace std;

class SectionDefinition
{
public:

	SectionDefinition(string secName);
	SectionDefinition(string secName, unsigned int offset, int size);
	~SectionDefinition();

	static int getLastSecId();

	void addLiteralToPool(Literal literal);

	bool checkIfLiteralIsInPool(int val);
	bool checkIfSymbolIsInPool(string sym);

	void setSectionSize(int size);
	int getSectionSize();

	size_t getSizeOfLitPool();
	void setAdressOfLiterals(int pc);

	Literal getLiteral(int val);
	Literal getLiteral(string val);
	int getLitValue(int index);

	list<RelocationTableEntry> getRelTable();

	int getSectionID();
	void setSectionID(int ID);

	unsigned int getSectionOffset();
	void setSectionOffset(unsigned int offset);

	void addSectionOffset(unsigned int offset);
	void addSectionSize(int size);

	list<char> getContent();

	void addSectionBinaryContent(char c);
	void setSectionBinaryContent(int index, char c);

	void addRelocationTableEntry(RelocationTableEntry rel);

	static void resetSectionID();

private:
	static int globalSecID;
	string secName;
	int secID, secSize, currLiteralIndex;
	list<char> content;
	list<RelocationTableEntry> relaTable;
	list<Literal> literalTable;
	unsigned int adress;
};
