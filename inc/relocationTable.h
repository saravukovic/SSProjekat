#pragma once
#include <string>

using namespace std;

class RelocationTableEntry
{
public:

	RelocationTableEntry(string type = "", string section = "", unsigned int offset = 0, int symId = 0, int addend = 0);
	~RelocationTableEntry();

	string getRelType();
	string getRelSection();
	unsigned int getRelOffset();
	int getSymbolId();
	int getAddend();

private:
	string type, section;
	unsigned int offset;
	int symbolId, addend;
};
