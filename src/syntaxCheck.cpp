#include "../inc/syntaxCheck.h"

SyntaxCheck::SyntaxCheck()
{
}

SyntaxCheck::~SyntaxCheck()
{
}

TypeOfAssemblerCode SyntaxCheck::getTypeOfAssemblerCode(string* line)
{
	TypeOfAssemblerCode type = COMMENT;
	smatch sm;
	regex comment = regex("^#.*");
	regex label = regex("^([a-zA-Z]*[_a-zA-Z0-9]*)( *):.*$");
	regex directive = regex("^\\..*");

	trimLine(line);

	if (regex_match(*line, sm, comment)) return COMMENT;
	if (regex_match(*line, sm, label)) return LABEL;
	if (regex_match(*line, sm, directive)) return DIRECTIVE;
	return INSTRUCTION;
}

TypeOfLabel SyntaxCheck::CheckLabelSintax(string* line, smatch* sm, int numOfPass)
{
	regex onlyLabel = regex("^([a-zA-Z]*[_a-zA-Z0-9]*):$");
	regex labelAndInstr = regex("^([a-zA-Z]*[_a-zA-Z0-9]*):(.*)$");
	TypeOfLabel retVal = LABELERROR;
	if(numOfPass == 1) trimLine(line);

	if (regex_match(*line, *sm, onlyLabel)) retVal = ONLYLABEL;
	else if (regex_match(*line, *sm, labelAndInstr)) retVal = LABELANDINSTR;

	return retVal;
}

TypeOfInstruction SyntaxCheck::CheckInstructionSintax(string* line, smatch* sm, int numOfPass)
{
	TypeOfInstruction retVal = INSTRERROR;

	string instr[NUMBEROFINSTRUCTIONS] = {
		"^(halt)$",
		"^(int)$",
		"^(iret)$",
		"^(call) (.*)$",
		"^(ret)$",
		"^(jmp) (.*)$",
		"^(beq) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp),(.*)$",
		"^(bne) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp),(.*)$",
		"^(bgt) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp),(.*)$",
		"^(push) %(r[0-9]|r1[0-5]|pc|sp)$",
		"^(pop) %(r[0-9]|r1[0-5]|pc|sp)$",
		"^(xchg) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(add) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(sub) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(mul) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(div) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(not) %(r[0-9]|r1[0-5]|pc|sp)$",
		"^(and) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(or) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(xor) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(shl) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(shr) %(r[0-9]|r1[0-5]|pc|sp),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(ld) (.*),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(st) %(r[0-9]|r1[0-5]|pc|sp),(.*)$",
		"^(csrrd) %(status|handler|cause),%(r[0-9]|r1[0-5]|pc|sp)$",
		"^(csrwr) %(r[0-9]|r1[0-5]|pc|sp),%(status|handler|cause)$"
	};

	if (numOfPass == 1) trimLine(line);

	for (unsigned int i = 0; i < NUMBEROFINSTRUCTIONS; i++)
	{
		if (regex_match(*line, *sm, regex(instr[i]))) {
			retVal = static_cast<TypeOfInstruction>(i);
			break;
		}
	}

	return retVal;
}


TypeOfDirective SyntaxCheck::CheckDirectiveSintax(string* line, smatch* sm, int numOfPass)
{
	TypeOfDirective retVal = ERRORDIR;

	if (numOfPass == 1) trimLine(line);

	string directive[6] = {
		"^\\.global ([a-zA-Z][_A-Za-z0-9]*(,[a-zA-Z][_A-Za-z0-9]*)*)$",
		"^\\.extern ([a-zA-Z][_A-Za-z0-9]*(,[a-zA-Z][_A-Za-z0-9]*)*)$",
		"^\\.section ([a-zA-Z][_A-Za-z0-9]*)$",
		"^\\.word ((-?[0-9]+|0x[A-F0-9]+|[a-zA-Z][_A-Za-z0-9]*)(,(-?[0-9]+|0x[A-F0-9]+|[a-zA-Z][_A-Za-z0-9]*))*)$",
		"^\\.skip (-?[0-9]+|0x[A-F0-9]+)$",
		"^\\.end$"
	};

	for (unsigned int i = 0; i < 6; i++)
	{
		if (regex_match(*line, *sm, regex(directive[i]))) {
			retVal = static_cast<TypeOfDirective>(i);
			break;
		}
	}

	return retVal;
}

TypeOfLiteral SyntaxCheck::CheckLiteralSintax(string literal, smatch* sm)
{
	TypeOfLiteral retVal = ERRORLIT;

	regex literalReg = regex("^(-?[0-9]+|0x[A-F0-9]+)$");
	regex symbolReg = regex("^([a-zA-Z]+[_a-zA-Z0-9]*)$");

	if (regex_match(literal, *sm, literalReg)) retVal = LITERAL;
	else if (regex_match(literal, *sm, symbolReg)) retVal = SYMBOL;

	return retVal;
}

TypeOfOperand SyntaxCheck::CheckOperandSintax(string operand, smatch* sm)
{
	TypeOfOperand retVal = ERROROP;

	string operandRegex[8] = {
		"^\\$(-?[0-9]+|0x[A-F0-9]+)$",
		"^\\$([a-zA-Z]+[_a-zA-Z0-9]*)$",
		"^(-?[0-9]+|0x[A-F0-9]+)$",
		"^([a-zA-Z]+[_a-zA-Z0-9]*)$",
		"^%(r[0-9]|r1[0-5]|pc|sp)$",
		"^\\[%(r[0-9]|r1[0-5]|pc|sp)\\]$",
		"^\\[%(r[0-9]|r1[0-5]|pc|sp) \\+ (-?[0-9]+|0x[A-F0-9]+)\\]$",
		"^\\[%(r[0-9]|r1[0-5]|pc|sp) \\+ ([a-zA-Z]+[_a-zA-Z0-9]*)\\]$"
	};

	for (unsigned int i = 0; i < NUMBEROFINSTRUCTIONS; i++)
	{
		if (regex_match(operand, *sm, regex(operandRegex[i]))) {
			retVal = static_cast<TypeOfOperand>(i);
			break;
		}
	}
	return retVal;
}

void SyntaxCheck::trimLine(string* line)
{
	*line = regex_replace(*line, oneLineComment, "");
	*line = regex_replace(*line, comment, "$1");
	*line = regex_replace(*line, comma, ",");
	*line = regex_replace(*line, colon, ":");
	*line = regex_replace(*line, whiteSpaces, "");
}
