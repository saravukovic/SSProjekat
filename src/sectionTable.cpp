#include "../inc/sectionTable.h"

int SectionDefinition::globalSecID = 0;

SectionDefinition::SectionDefinition(string secName)
{
	this->secName = secName;
	this->secSize = 0;
	this->currLiteralIndex = 0;
	this->secID = globalSecID++;
	this->adress = 0;
}

SectionDefinition::SectionDefinition(string secName, unsigned int offset, int size)
{
	this->secName = secName;
	this->secSize = size;
	this->currLiteralIndex = 0;
	this->secID = globalSecID++;
	this->adress = offset;
}

SectionDefinition::~SectionDefinition()
{
}

int SectionDefinition::getLastSecId()
{
	return globalSecID - 1;
}

void SectionDefinition::addLiteralToPool(Literal literal)
{
	literalTable.push_back(literal);
}

bool SectionDefinition::checkIfLiteralIsInPool(int val)
{
	for (Literal literal : literalTable)
	{
		if (literal.getLiteralValue() == val) return true;
	}
	return false;
}

bool SectionDefinition::checkIfSymbolIsInPool(string sym)
{

	return false;
}

void SectionDefinition::setSectionSize(int size)
{
	this->secSize = size;
}

int SectionDefinition::getSectionSize()
{
	return this->secSize;
}

size_t SectionDefinition::getSizeOfLitPool()
{
	return literalTable.size();
}

void SectionDefinition::setAdressOfLiterals(int pc)
{
	for (Literal &l : literalTable) {
		l.setLiteralAdress(pc);
		pc += 4;
	}
}

Literal SectionDefinition::getLiteral(int val)
{
	for (Literal literal : literalTable)
	{
		if (literal.getLiteralValue() == val) return literal;
	}
	return Literal();
}

Literal SectionDefinition::getLiteral(string val)
{
	for (Literal literal : literalTable)
	{
		if (literal.getLiteralName() == val) return literal;
	}
	return Literal();
}

int SectionDefinition::getLitValue(int index)
{
	int i = 0;
	for (Literal literal : literalTable)
	{
		if (i == index) return literal.getLiteralValue();
		i++;
	}
	return 0;
}

list<RelocationTableEntry> SectionDefinition::getRelTable()
{
	return this->relaTable;
}

int SectionDefinition::getSectionID()
{
	return this->secID;
}

void SectionDefinition::setSectionID(int ID)
{
	secID = ID;
}

unsigned int SectionDefinition::getSectionOffset()
{
	return adress;
}

void SectionDefinition::setSectionOffset(unsigned int offset)
{
	adress = offset;
}

void SectionDefinition::addSectionOffset(unsigned int offset)
{
	this->adress += offset;
}

void SectionDefinition::addSectionSize(int size)
{
	this->secSize += size;
}

list<char> SectionDefinition::getContent()
{
	return this->content;
}

void SectionDefinition::addSectionBinaryContent(char c)
{
	content.push_back(c);
}

void SectionDefinition::setSectionBinaryContent(int index, char c)
{
	int i = 0;
	
	for (char &con : content) {
		if (i == index) {
			con = c;
			break;
		}
		i++;
	}

}

void SectionDefinition::addRelocationTableEntry(RelocationTableEntry rel)
{
	relaTable.push_back(rel);
}

void SectionDefinition::resetSectionID()
{
	globalSecID = 0;
}
