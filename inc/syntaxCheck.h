#pragma once

#include <string>
#include <regex>

using namespace std;

#define NUMBEROFINSTRUCTIONS	26

enum TypeOfAssemblerCode
{
	COMMENT,
	LABEL,
	DIRECTIVE,
	INSTRUCTION
};

enum TypeOfLabel
{
	ONLYLABEL,
	LABELANDINSTR,
	LABELERROR
};

enum TypeOfInstruction
{
	HALT,
	INT,
	IRET,
	CALL,
	RET,
	JMP,
	BEQ,
	BNE,
	BGT,
	PUSH,
	POP,
	XCHG,
	ADD,
	SUB,
	MUL,
	DIV,
	NOT,
	AND,
	OR,
	XOR,
	SHL,
	SHR,
	LD,
	ST,
	CSRRD,
	CSRWR,
	INSTRERROR
};

enum TypeOfDirective
{
	GLOBALDIR,
	EXTERNDIR,
	SECTIONDIR,
	WORDDIR,
	SKIPDIR,
	ENDDIR,
	ERRORDIR
};

enum TypeOfLiteral
{
	LITERAL,
	SYMBOL,
	ERRORLIT
};

enum TypeOfOperand
{
	LITERALVAL,
	SYMBOLVAL,
	LITERALMEM,
	SYMBOLMEM,
	REGVAL,
	REGMEM,
	REGLITMEM,
	REGSYMMEM,
	ERROROP
};

class SyntaxCheck
{
public:
	SyntaxCheck();
	~SyntaxCheck();

	TypeOfAssemblerCode getTypeOfAssemblerCode(string* line);
	TypeOfLabel CheckLabelSintax(string* line, smatch* sm, int numOfPass);
	TypeOfInstruction CheckInstructionSintax(string* line, smatch* sm, int numOfPass);
	TypeOfDirective CheckDirectiveSintax(string* line, smatch* sm, int numOfPass);

	TypeOfLiteral CheckLiteralSintax(string literal, smatch* sm);
	TypeOfOperand CheckOperandSintax(string operand, smatch* sm);


private:
	void trimLine(string* line);

	regex whiteSpaces = regex("^\\s+|\\s+$|\\s+(?=\\s)/g");
	regex oneLineComment = regex("^#.*");
	regex comment = regex("^([^#]*)#.*");
	regex comma = regex(" *, *");
	regex colon = regex(" *: *");


};
