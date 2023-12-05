#include "../inc/relocationTable.h"

RelocationTableEntry::RelocationTableEntry(string type, string section, unsigned int offset, int symId, int addend)
{
	this->type = type;
	this->section = section;
	this->offset = offset;
	this->symbolId = symId;
	this->addend = addend;
}

RelocationTableEntry::~RelocationTableEntry()
{
}

string RelocationTableEntry::getRelType()
{
	return this->type;
}

string RelocationTableEntry::getRelSection()
{
	return this->section;
}

unsigned int RelocationTableEntry::getRelOffset()
{
	return this->offset;
}

int RelocationTableEntry::getSymbolId()
{
	return this->symbolId;
}

int RelocationTableEntry::getAddend()
{
	return this->addend;
}
