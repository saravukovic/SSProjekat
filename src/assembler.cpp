#include "../inc/assembler.h"
#pragma warning( disable : 4309 )
#pragma warning( disable : 4312 )

Assembler::Assembler(string input, string output) : pc(0), numOfLine(0)
{
	inputFileName = input;
	outputFileName = output;
	parser = SyntaxCheck();
	ldLitMem = false;
	assembling = true;
}

Assembler::~Assembler()
{
}

void Assembler::firstPass()
{
	pc = 0;
	string line;

	ifstream file(inputFile);

	if (!file.is_open()) {
		cout << "Greska! Dokument sa imenom " << inputFile << " nije moguce otvoriti" << endl;
		exit(-1);
	}

	while (!file.eof() and assembling) {
		getline(file, line);

		TypeOfAssemblerCode type = parser.getTypeOfAssemblerCode(&line);

		if (line.empty()) continue;

		switch (type)
		{
		case COMMENT:
			break;
		case LABEL:
			labelInFirstPass(&line);
			break;
		case DIRECTIVE:
			directiveInFirstPass(&line);
			break;
		case INSTRUCTION:
			instructionInFirstPass(&line);
			break;
		default:
			cout << "Greska! Linija " << line << " ne moze da se isparsira." << endl;
			break;
		}
		formattedInputFile.push_back(line);
	}
}

void Assembler::secondPass()
{
	currSection = "";
	currSectionNum = 0;
	pc = 0;
	for (string line : formattedInputFile)
	{
		TypeOfAssemblerCode type = parser.getTypeOfAssemblerCode(&line);

		switch (type)
		{
		case COMMENT:
			break;
		case LABEL:
			labelInSecondPass(&line);
			break;
		case DIRECTIVE:
			directiveInSecondPass(&line);
			break;
		case INSTRUCTION:
			instructionInSecondPass(&line);
			break;
		default:
			cout << "Greska! Linija " << line << " ne moze da se isparsira." << endl;
			break;
		}
	}
}

void Assembler::assembler()
{
	string path = "C:\\Users\\Sara\\Desktop\\ssproba\\";
	inputFile = path + inputFileName;
	outputFile = path + outputFileName;
	SectionDefinition::resetSectionID();
	SymbolDefinition::resetSymID();
	firstPass();
	secondPass();
	createELF();
	createBinary();
}

/*-------------------functions for first pass-------------------*/
void Assembler::labelInFirstPass(string* line)
{
	smatch sm;

	//check if label is in proper format and format the line
	TypeOfLabel typeOfLabel = parser.CheckLabelSintax(line, &sm, 1);

	if (typeOfLabel == LABELERROR) {
		cout << "Greska! Labela nije napisana u odredjenom formatu." << endl;
		exit(-1);
	}

	string label = sm.str(1);

	if (currSection == "") {
		cout << "Greska! Labela " << label << " mora biti definisana u okviru neke sekcije!" << endl;
		exit(-1);
	}

	if (symbolTable.count(label) == 0) {
		SymbolDefinition sym(label, currSection, currSectionNum, NOTYPE, LOCAL, pc, false);
		symbolTable.insert({ label, sym });
	}
	else
	{
		SymbolDefinition& sym = symbolTable.at(label);
		if (sym.isSymbolDefined()) {
			cout << "Simbol " << label << " je vec definisan." << endl;
			exit(-1);
		}

		sym.defineSymbol(pc, currSection, currSectionNum);
	}

	string instr;

	if (typeOfLabel == LABELANDINSTR) {
		instr = sm.str(2);
		instructionInFirstPass(&instr);
	}

}

void Assembler::instructionInFirstPass(string* instr)
{
	smatch sm;
	TypeOfInstruction typeOfInstr = parser.CheckInstructionSintax(instr, &sm, 1);

	switch (typeOfInstr)
	{
		/*-------------------instructions with no operands-------------------*/
	case HALT:
	case INT:
	case RET:
		pc += 4;
		break;
	case IRET:
		pc += 12;
		break;
		/*-------------------instructions with one operand-------------------*/
	case CALL:
	case JMP:
		checkAndAddToLiteralTable(sm.str(2));
		pc += 4;
		break;
		/*-------------------instructions with two gprs and one operand-------------------*/
	case BEQ:
	case BNE:
	case BGT:
		checkAndAddToLiteralTable(sm.str(3));
		pc += 4;
		break;
		/*-------------------instructions with one gpr-------------------*/
	case PUSH:
	case POP:
		/*-------------------instructions with two gprs-------------------*/
	case XCHG:
	case ADD:
	case SUB:
	case MUL:
	case DIV:
		/*-------------------instruction with one gpr-------------------*/
	case NOT:
		/*-------------------instructions with two gprs-------------------*/
	case AND:
	case OR:
	case XOR:
	case SHL:
	case SHR:
		pc += 4;
		break;
		/*-------------------instruction with one gpr and one operand-------------------*/
	case LD:
		checkOperandAndAddToLiteralTable(sm.str(2));
		if (ldLitMem) pc += 8;
		else pc += 4;
		break;
	case ST:
		checkOperandAndAddToLiteralTable(sm.str(3));
		pc += 4;
		break;
		/*-------------------instruction with one gpr and one csr-------------------*/
	case CSRRD:
	case CSRWR:
		pc += 4;
		break;
	case INSTRERROR:
		cout << "Sintaksna greska: instrukcija " << *instr << "nije u odgovarajucem formatu" << endl;
		break;
	default:
		break;
	}
}

void Assembler::directiveInFirstPass(string* line)
{
	smatch sm;
	TypeOfDirective type = parser.CheckDirectiveSintax(line, &sm, 1);

	switch (type)
	{
	case GLOBALDIR:
	case EXTERNDIR:
	case WORDDIR:
		checkDirectiveListOfSymbols(sm.str(1), type);
		break;
	case SKIPDIR:
	{
		unsigned int val = 0;
		string lit = sm.str(1);
		if (lit[0] == '0' and lit[1] == 'x') {
			lit.erase(0, 2);
			val = stoul(lit, 0, 16);
		}
		else
		{
			val = stoul(lit, 0, 10);
		}
		/* mislim da mi ovo ipak mozda i ne treba jer sam stavila da je val uint */
		//if (val < 0) {
		//	cout << "Greska! Skip directiva mora imati pozitivan literal." << endl;
		//	exit(-1);
		//}
		pc += val;
		break;
	}
	case SECTIONDIR:
	case ENDDIR:
	{
		string secName = sm.str(1);
		if (type == ENDDIR) {
			secName = "";
			assembling = false;
		}
		secAndEndDirective(secName, type);
		break;
	}
	case ERRORDIR:
		cout << "Greska! Direktiva nije pravilno definisana." << endl;
		exit(-1);
		break;
	default:
		break;
	}

}

void Assembler::checkAndAddToLiteralTable(string operand)
{
	smatch sm;
	TypeOfLiteral type = parser.CheckLiteralSintax(operand, &sm);

	if (type == SYMBOL) {

	}
	else if (type == LITERAL) {
		unsigned int val = 0;
		if (operand[0] == '0' and operand[1] == 'x') {
			operand.erase(0, 2);
			val = stoul(operand, 0, 16);
		}
		else
		{
			val = stoul(operand, 0, 10);
		}

		if (val > MAXLITVAL) {
			SectionDefinition* currSec = sectionTable.at(currSection);

			if (!currSec->checkIfLiteralIsInPool(val)) currSec->addLiteralToPool(Literal("", 0, val));
		}
	}
	else
	{
		cout << "Sintaksna greska: literal/simbol " << operand << "nije u odgovarajucem formatu" << endl;
	}
}

void Assembler::checkOperandAndAddToLiteralTable(string operand)
{
	ldLitMem = false;
	smatch sm;
	TypeOfOperand type = parser.CheckOperandSintax(operand, &sm);

	if (type == LITERALVAL || type == LITERALMEM) {
		operand = sm.str(1);
		unsigned int val = 0;
		if (operand[0] == '0' and operand[1] == 'x') {
			operand.erase(0, 2);
			val = stoul(operand, 0, 16);
		}
		else
		{
			val = stoul(operand, 0, 10);
		}

		if (val > MAXLITVAL) {
			SectionDefinition* currSec = sectionTable.at(currSection);

			if (!currSec->checkIfLiteralIsInPool(val)) currSec->addLiteralToPool(Literal("", 0, val));
			if (type == LITERALMEM) ldLitMem = true;
		}

	}
	else if (type == SYMBOLMEM)
	{
		ldLitMem = true;
	}
	else if (type == REGLITMEM)
	{
		operand = sm.str(2);
		unsigned int val = 0;
		if (operand[0] == '0' and operand[1] == 'x') {
			operand.erase(0, 2);
			val = stoul(operand, 0, 16);
		}
		else
		{
			val = stoul(operand, 0, 10);
		}

		if (val > MAXLITVAL) {
			cout << "Greska: literal " << operand << "nije u odgovarajucem opsegu" << endl;
		}
	}
	else if (type == ERROROP)
	{
		cout << "Sintaksna greska: literal/simbol " << operand << " nije u odgovarajucem formatu" << endl;
	}

}

void Assembler::checkDirectiveListOfSymbols(string listSym, TypeOfDirective dir)
{
	list<string> symbols;

	while (listSym.size()) {
		size_t i = listSym.find(",");
		if (i == string::npos) {
			symbols.push_back(listSym);
			listSym = "";
		}
		else {
			string s = listSym.substr(0, i);
			symbols.push_back(s);
			listSym.erase(0, i + 1); //delete delimiter also (,)
		}
	}

	if (dir == GLOBALDIR || dir == EXTERNDIR) {
		for (string sym : symbols)
		{
			if (symbolTable.count(sym)) {
				SymbolDefinition& symInSymTable = symbolTable.at(sym);

				if (symInSymTable.isSymbolExtern()) {
					cout << "Greska! Simbol " << sym << " je vec deklarisan kao eksterni simbol" << endl;
					exit(-1);
				}

				if (dir == EXTERNDIR && symInSymTable.isSymbolDefined()) {
					cout << "Greska! Simbol " << sym << " je vec definisan" << endl;
					exit(-1);
				}

				if (((!symInSymTable.isSymbolDefined() && symInSymTable.getSymbolBinding() == GLOBAL) && dir == GLOBALDIR) || (dir == EXTERNDIR)) {
					cout << "Greska! Simbol " << sym << " je vec deklarisan kao globalni simbol" << endl;
					exit(-1);
				}

			}
			else {
				symbolTable.insert({ sym, SymbolDefinition(sym, "", 0, NOTYPE, GLOBAL, 0, (dir == EXTERNDIR)) });
			}
		}
	}
	else if (dir == WORDDIR) {
		if (currSection == "") {
			cout << "Greska! Direktiva word mora da se nadje u okviru neke sekcije." << endl;
			exit(-1);
		}
		pc += static_cast<unsigned int>(symbols.size()) * 4;
	}

}

void Assembler::secAndEndDirective(string secName, TypeOfDirective dir)
{
	if (sectionTable.size()) {
		SectionDefinition* curSec = sectionTable.at(currSection);

		if (pc > MAXLITVAL) {
			cout << "Greska! Sekcija " << currSection << " je veca od 2047!" << endl;
			exit(-1);
		}
		if (curSec->getSizeOfLitPool() > 0) {
			curSec->setAdressOfLiterals(pc);
			pc += static_cast<unsigned int>(curSec->getSizeOfLitPool()) * 4;
		}

		curSec->setSectionSize(pc);
	}

	if (dir == SECTIONDIR) {
		if (symbolTable.count(secName)) {
			cout << "Greska! Sekcija " << secName << " je vec deklarisana kao simbol!" << endl;
			exit(-1);
		}
		sectionTable.insert({ secName, new SectionDefinition(secName) });
		symbolTable.insert({ secName, SymbolDefinition(secName, secName, SectionDefinition::getLastSecId(), SECTION, LOCAL, pc, false) });
	}

	currSection = secName;
	currSectionNum = SectionDefinition::getLastSecId();
	pc = 0;
}

/*-------------------functions for second pass-------------------*/

void Assembler::labelInSecondPass(string* line)
{
	smatch sm;
	string instr;

	//get type of label
	TypeOfLabel typeOfLabel = parser.CheckLabelSintax(line, &sm, 2);

	if (typeOfLabel == ONLYLABEL) {
		return;
	}

	instr = sm.str(2);
	instructionInSecondPass(&instr);
}

void Assembler::instructionInSecondPass(string* instr)
{
	smatch sm;
	TypeOfInstruction typeOfInstr = parser.CheckInstructionSintax(instr, &sm, 2);
	SectionDefinition* currSec = sectionTable.at(currSection);

	switch (typeOfInstr)
	{
	case HALT:
	{
		for (unsigned int i = 0; i < 4; i++)
		{
			currSec->addSectionBinaryContent(0x00);
		}
		pc += 4;
		break;
	}
	case INT:
	{
		currSec->addSectionBinaryContent(0x10);
		for (unsigned int i = 0; i < 3; i++)
		{
			currSec->addSectionBinaryContent(0x00);
		}
		pc += 4;
		break;
	}
	case IRET:
	{
		currSec->addSectionBinaryContent(0x96);
		currSec->addSectionBinaryContent(0x0E);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x04);

		currSec->addSectionBinaryContent(0x92);
		currSec->addSectionBinaryContent(0xFE);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x00);

		currSec->addSectionBinaryContent(0x91);
		currSec->addSectionBinaryContent(0xEE);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x08);

		pc += 12;
		break;
	}
	case CALL:
	{
		char literal[3] = { 0 };
		getSymOrLitAdrSecPass(sm.str(2), literal);

		if (ldLitMem) {
			currSec->addSectionBinaryContent(0x21);
			currSec->addSectionBinaryContent(0xF0);
		}
		else {
			currSec->addSectionBinaryContent(0x20);
			currSec->addSectionBinaryContent(0x00);
		}
		currSec->addSectionBinaryContent(literal[0]);
		currSec->addSectionBinaryContent((literal[1] << 4) | literal[2]);

		pc += 4;
		break;
	}
	case RET:
	{
		currSec->addSectionBinaryContent(0x93);
		currSec->addSectionBinaryContent(0xFE);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x04);
		pc += 4;
		break;
	}
	case JMP:
	{
		char literal[3] = { 0 };
		getSymOrLitAdrSecPass(sm.str(2), literal);

		if (ldLitMem) {
			currSec->addSectionBinaryContent(0x38);
			currSec->addSectionBinaryContent(0xF0);
		}
		else {
			currSec->addSectionBinaryContent(0x30);
			currSec->addSectionBinaryContent(0x00);
		}
		currSec->addSectionBinaryContent(literal[0]);
		currSec->addSectionBinaryContent((literal[1] << 4) | literal[2]);

		pc += 4;
		break;
	}
	case BEQ:
	{
		char r1 = regToChar(sm.str(2));
		char r2 = regToChar(sm.str(3));

		char literal[3] = { 0 };
		getSymOrLitAdrSecPass(sm.str(4), literal);

		if (ldLitMem) {
			currSec->addSectionBinaryContent(0x39);
			currSec->addSectionBinaryContent((0xF0) | r1);
		}
		else {
			currSec->addSectionBinaryContent(0x31);
			currSec->addSectionBinaryContent(r1);
		}
		currSec->addSectionBinaryContent((r2 << 4) | literal[0]);
		currSec->addSectionBinaryContent((literal[1] << 4) | literal[2]);

		pc += 4;
		break;
	}
	case BNE:
	{
		char r1 = regToChar(sm.str(2));
		char r2 = regToChar(sm.str(3));

		char literal[3] = { 0 };
		getSymOrLitAdrSecPass(sm.str(4), literal);

		if (ldLitMem) {
			currSec->addSectionBinaryContent(0x3A);
			currSec->addSectionBinaryContent((0xF0) | r1);
		}
		else {
			currSec->addSectionBinaryContent(0x32);
			currSec->addSectionBinaryContent(r1);
		}
		currSec->addSectionBinaryContent((r2 << 4) | literal[0]);
		currSec->addSectionBinaryContent((literal[1] << 4) | literal[2]);

		pc += 4;
		break;
	}
	case BGT:
	{
		char r1 = regToChar(sm.str(2));
		char r2 = regToChar(sm.str(3));

		char literal[3] = { 0 };
		getSymOrLitAdrSecPass(sm.str(4), literal);

		if (ldLitMem) {
			currSec->addSectionBinaryContent(0x3B);
			currSec->addSectionBinaryContent((0xF0) | r1);
		}
		else {
			currSec->addSectionBinaryContent(0x33);
			currSec->addSectionBinaryContent(r1);
		}
		currSec->addSectionBinaryContent((r2 << 4) | literal[0]);
		currSec->addSectionBinaryContent((literal[1] << 4) | literal[2]);

		pc += 4;
		break;
	}
	case PUSH:
	{
		char reg = regToChar(sm.str(2));

		currSec->addSectionBinaryContent(0x81);
		currSec->addSectionBinaryContent(0xE0);
		currSec->addSectionBinaryContent((reg << 4) | 0x0F);
		currSec->addSectionBinaryContent(0xFC);
		pc += 4;
		break;
	}
	case POP:
	{
		char reg = regToChar(sm.str(2));

		currSec->addSectionBinaryContent(0x93);
		currSec->addSectionBinaryContent((reg << 4) | 0x0E);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x04);
		pc += 4;
		break;
	}
	case XCHG:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x80);
		currSec->addSectionBinaryContent(rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case ADD:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x50);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case SUB:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x51);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case MUL:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x52);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case DIV:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x53);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case NOT:
	{
		char reg = regToChar(sm.str(2));

		currSec->addSectionBinaryContent(0x60);
		currSec->addSectionBinaryContent((reg << 4) | reg);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case AND:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x61);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case OR:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x62);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case XOR:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x63);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case SHL:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x70);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case SHR:
	{
		char rs = regToChar(sm.str(2));
		char rd = regToChar(sm.str(3));

		currSec->addSectionBinaryContent(0x71);
		currSec->addSectionBinaryContent((rd << 4) | rd);
		currSec->addSectionBinaryContent(rs << 4);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case LD:
	{
		loadInstrHandler(sm.str(2), sm.str(3));
		break;
	}
	case ST:
	{
		storeInstrHandler(sm.str(3), sm.str(2));
		break;
	}
	case CSRRD:
	{
		char csr = 0;

		if (sm.str(2) == "status") csr = 0x00;
		else if (sm.str(2) == "handler") csr = 0x01;
		else if (sm.str(2) == "cause") csr = 0x02;

		char gpr = regToChar(sm.str(3));
		currSec->addSectionBinaryContent(0x90);
		currSec->addSectionBinaryContent((gpr << 4) | csr);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case CSRWR:
	{
		char csr = 0;

		if (sm.str(3) == "status") csr = 0x00;
		else if (sm.str(3) == "handler") csr = 0x01;
		else if (sm.str(3) == "cause") csr = 0x02;

		char gpr = regToChar(sm.str(2));

		currSec->addSectionBinaryContent(0x90);
		currSec->addSectionBinaryContent((gpr << 4) | csr);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x00);
		pc += 4;
		break;
	}
	case INSTRERROR:
		break;
	default:
		break;
	}

}

void Assembler::directiveInSecondPass(string* line)
{
	smatch sm;
	TypeOfDirective type = parser.CheckDirectiveSintax(line, &sm, 2);

	switch (type)
	{
	case GLOBALDIR:
	case EXTERNDIR:
		break;
	case SECTIONDIR:
	case ENDDIR:
	{
		string secName = sm.str(1);
		if (type == ENDDIR) {
			secName = "";
			assembling = false;
		}
		secAndEndDirectiveSecondPass(secName, type);
		break;
	}
	case WORDDIR:
	{
		string listSym = sm.str(1);
		list<string> symbols;

		while (listSym.size()) {
			size_t i = listSym.find(",");
			if (i == string::npos) {
				symbols.push_back(listSym);
				listSym = "";
			}
			else {
				string s = listSym.substr(0, i);
				symbols.push_back(s);
				listSym.erase(0, i + 1); //delete delimiter also (,)
			}
		}

		for (string sym : symbols) {
			smatch sm;
			TypeOfLiteral type = parser.CheckLiteralSintax(sym, &sm);
			SectionDefinition* currSec = sectionTable.at(currSection);

			if (type == LITERAL) {
				unsigned int val = 0;
				unsigned int adr = 0;
				if (sym[0] == '0' and sym[1] == 'x') {
					sym.erase(0, 2);
					val = stoul(sym, 0, 16);
				}
				else
				{
					val = stoul(sym, 0, 10);
				}

				currSec->addSectionBinaryContent((char)(val & 0xFF));
				val >>= 8;
				currSec->addSectionBinaryContent((char)(val & 0xFF));
				val >>= 8;
				currSec->addSectionBinaryContent((char)(val & 0xFF));
				val >>= 8;
				currSec->addSectionBinaryContent((char)(val & 0xFF));

			}
			else
			{
				if (symbolTable.count(sym)) {
					SymbolDefinition symbol = symbolTable.at(sym);
					if (symbol.getSymbolsSection() == currSection) {
						unsigned int adr = symbol.getSymAdress();
						currSec->addSectionBinaryContent((char)(adr & 0xFF));
						adr >>= 8;
						currSec->addSectionBinaryContent((char)(adr & 0xFF));
						adr >>= 8;
						currSec->addSectionBinaryContent((char)(adr & 0xFF));
						adr >>= 8;
						currSec->addSectionBinaryContent((char)(adr & 0xFF));
					}
					else
					{
						RelocationTableEntry rel;
						if (symbol.getSymbolBinding() == LOCAL) {
							rel = RelocationTableEntry("R_ABSREL_32", currSection, currSec->getLiteral(sym).getLiteralAdress(), symbol.getSecNum(), symbol.getSymAdress());
						}
						else
						{
							rel = RelocationTableEntry("R_ABSREL_32", currSection, currSec->getLiteral(sym).getLiteralAdress(), symbol.getSymID(), 0);
						}
						currSec->addRelocationTableEntry(rel);
					}
				}
				else
				{
					cout << "Greska, simbol " << sym << " ne postoji." << endl;
					exit(-1);
				}
			}
			pc += 4;
		}

		break;
	}
	case SKIPDIR:
	{
		unsigned int val = 0;
		unsigned int adr = 0;
		SectionDefinition* currSec = sectionTable.at(currSection);
		string lit = sm.str(1);
		if (lit[0] == '0' and lit[1] == 'x') {
			lit.erase(0, 2);
			val = stoul(lit, 0, 16);
		}
		else
		{
			val = stoul(lit, 0, 10);
		}

		while (val) {
			currSec->addSectionBinaryContent(0x00);
			pc++;
			val--;
		}

		break;
	}
	case ERRORDIR:
		break;
	default:
		break;
	}
}

void Assembler::getSymOrLitAdrSecPass(string operand, char* adress)
{
	ldLitMem = false;
	smatch sm;
	SectionDefinition* currSec = sectionTable.at(currSection);
	TypeOfLiteral type = parser.CheckLiteralSintax(operand, &sm);

	if (type == SYMBOL) {
		if (symbolTable.count(operand)) {
			SymbolDefinition sym = symbolTable.at(operand);
			if (sym.isSymbolDefined()) {
				if (sym.getSymbolsSection() == currSection) {
					int val = 0;
					unsigned int adr = sym.getSymAdress();
					adress[0] = adr >> 8;
					adress[1] = (adr >> 4) & 0x0F;
					adress[2] = adr & 0x0F;
				}
				else
				{
					RelocationTableEntry rel;
					int litAdr = currSec->getSectionSize();

					currSec->addLiteralToPool(Literal(operand, litAdr, 0));

					if (sym.getSymbolBinding() == LOCAL) {
						rel = RelocationTableEntry("R_ABSREL_32", currSection, litAdr, sym.getSecNum(), sym.getSymAdress());
					}
					else
					{
						rel = RelocationTableEntry("R_ABSREL_32", currSection, litAdr, sym.getSymID(), 0);
					}
					currSec->setSectionSize(litAdr + 4);
					currSec->addRelocationTableEntry(rel);

					int displ = litAdr - pc - 4;

					adress[0] = displ >> 8;
					adress[1] = (displ >> 4) & 0x0F;
					adress[2] = displ & 0x0F;
					ldLitMem = true;
				}
			}
			else
			{
				int litAdr = currSec->getSectionSize();
				RelocationTableEntry rel = RelocationTableEntry("R_ABSREL_32", currSection, litAdr, sym.getSymID(), 0);

				currSec->addLiteralToPool(Literal(operand, litAdr, 0));
				currSec->setSectionSize(litAdr + 4);
				currSec->addRelocationTableEntry(rel);

				int displ = litAdr - pc - 4;

				adress[0] = displ >> 8;
				adress[1] = (displ >> 4) & 0x0F;
				adress[2] = displ & 0x0F;
				ldLitMem = true;
			}
		}
		else
		{
			SymbolDefinition sym = SymbolDefinition(operand, currSection, currSectionNum, NOTYPE, LOCAL, pc, true);

			int litAdr = currSec->getSectionSize();

			RelocationTableEntry rel = RelocationTableEntry("R_ABSREL_32", currSection, litAdr, sym.getSymID(), 0);
			currSec->addRelocationTableEntry(rel);

			currSec->addLiteralToPool(Literal(operand, litAdr, 0));
			currSec->setSectionSize(litAdr + 4);

			int displ = litAdr - pc - 4;

			adress[0] = displ >> 8;
			adress[1] = (displ >> 4) & 0x0F;
			adress[2] = displ & 0x0F;
			ldLitMem = true;
		}

	}
	else if (type == LITERAL) {
		unsigned int val = 0;
		unsigned int adr = 0;
		if (operand[0] == '0' and operand[1] == 'x') {
			operand.erase(0, 2);
			val = stoul(operand, 0, 16);
		}
		else
		{
			val = stoul(operand, 0, 10);
		}

		if (val > 2047) {
			if (currSec->checkIfLiteralIsInPool(val)) {
				adr = currSec->getLiteral(val).getLiteralAdress() - pc - 4;
				ldLitMem = true;
			}
		}
		else
		{
			adr = val;
		}

		adress[0] = adr >> 8;
		adress[1] = (adr >> 4) & 0x0F;
		adress[2] = adr & 0x0F;
	}

}

void Assembler::loadInstrHandler(string operand, string reg)
{
	ldLitMem = false;
	smatch sm;
	TypeOfOperand type = parser.CheckOperandSintax(operand, &sm);
	SectionDefinition* currSec = sectionTable.at(currSection);
	char rgd = regToChar(reg);
	char adress[3] = { 0 };

	switch (type)
	{
	case LITERALVAL:
	case SYMBOLVAL:
	{
		operand.erase(0, 1);
		getSymOrLitAdrSecPass(operand, adress);

		if (ldLitMem) {
			currSec->addSectionBinaryContent(0x92);
			currSec->addSectionBinaryContent((rgd << 4) | 0x0F);
			currSec->addSectionBinaryContent(adress[0]);
			currSec->addSectionBinaryContent((adress[1] << 4) | adress[2]);
		}
		else
		{
			currSec->addSectionBinaryContent(0x91);
			currSec->addSectionBinaryContent(rgd << 4);
			currSec->addSectionBinaryContent(adress[0]);
			currSec->addSectionBinaryContent((adress[1] << 4) | adress[2]);
		}

		pc += 4;
		break;
	}
	case LITERALMEM:
	case SYMBOLMEM:
	{
		getSymOrLitAdrSecPass(operand, adress);

		if (ldLitMem) {
			currSec->addSectionBinaryContent(0x92);
			currSec->addSectionBinaryContent((rgd << 4) | 0x0F);
			currSec->addSectionBinaryContent(adress[0]);
			currSec->addSectionBinaryContent((adress[1] << 4) | adress[2]);

			currSec->addSectionBinaryContent(0x92);
			currSec->addSectionBinaryContent((rgd << 4) | rgd);
			currSec->addSectionBinaryContent(0x00);
			currSec->addSectionBinaryContent(0x00);

			pc += 8;
		}
		else
		{
			currSec->addSectionBinaryContent(0x92);
			currSec->addSectionBinaryContent(rgd << 4);
			currSec->addSectionBinaryContent(adress[0]);
			currSec->addSectionBinaryContent((adress[1] << 4) | adress[2]);

			pc += 4;
		}
		break;
	}
	case REGVAL:
	{
		char rgs = regToChar(operand);
		currSec->addSectionBinaryContent(0x91);
		currSec->addSectionBinaryContent((rgd << 4) | rgs);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x00);

		pc += 4;
		break;
	}
	case REGMEM:
	{
		char rgs = regToChar(operand);
		currSec->addSectionBinaryContent(0x92);
		currSec->addSectionBinaryContent((rgd << 4) | rgs);
		currSec->addSectionBinaryContent(0x00);
		currSec->addSectionBinaryContent(0x00);

		pc += 4;
		break;
	}
	case REGLITMEM:
	{
		char rgs = regToChar(sm.str(1));
		string literal = sm.str(2);

		unsigned int adr = 0;

		if (literal[0] == '0' and literal[1] == 'x') {
			literal.erase(0, 2);
			adr = stoul(literal, 0, 16);
		}
		else
		{
			adr = stoul(literal, 0, 10);
		}

		adress[0] = adr >> 8;
		adress[1] = (adr >> 4) & 0x0F;
		adress[2] = adr & 0x0F;


		currSec->addSectionBinaryContent(0x92);
		currSec->addSectionBinaryContent((rgd << 4) | rgs);
		currSec->addSectionBinaryContent(adress[0]);
		currSec->addSectionBinaryContent((adress[1] << 4) | adress[2]);

		pc += 4;
		break;
	}
	case REGSYMMEM:
	{
		cout << "Greska: simbol " << operand << "nije u odgovarajucem opsegu" << endl;
		break;
	}
	case ERROROP:
		break;
	default:
		break;
	}

}

void Assembler::storeInstrHandler(string operand, string reg)
{
	ldLitMem = false;
	smatch sm;
	TypeOfOperand type = parser.CheckOperandSintax(operand, &sm);
	SectionDefinition* currSec = sectionTable.at(currSection);
	char rgs = regToChar(reg);
	char adress[3] = { 0 };

	switch (type)
	{
	case LITERALMEM:
	case SYMBOLMEM:
	{
		getSymOrLitAdrSecPass(operand, adress);

		if (ldLitMem) {
			currSec->addSectionBinaryContent(0x82);
			currSec->addSectionBinaryContent(0xF0);
			currSec->addSectionBinaryContent((rgs << 4) | adress[0]);
			currSec->addSectionBinaryContent((adress[1] << 4) | adress[2]);

		}
		else
		{
			currSec->addSectionBinaryContent(0x80);
			currSec->addSectionBinaryContent(0x00);
			currSec->addSectionBinaryContent((rgs << 4) | adress[0]);
			currSec->addSectionBinaryContent((adress[1] << 4) | adress[2]);
		}
		pc += 4;
		break;
	}
	case REGMEM:
	{
		char rgd = regToChar(operand);
		currSec->addSectionBinaryContent(0x80);
		currSec->addSectionBinaryContent(rgd << 4);
		currSec->addSectionBinaryContent(rgs << 4);
		currSec->addSectionBinaryContent(0x00);

		pc += 4;
		break;
	}
	case REGLITMEM:
	{
		char rgd = regToChar(sm.str(1));
		string literal = sm.str(2);

		unsigned int adr = 0;

		if (literal[0] == '0' and literal[1] == 'x') {
			literal.erase(0, 2);
			adr = stoul(literal, 0, 16);
		}
		else
		{
			adr = stoul(literal, 0, 10);
		}

		adress[0] = adr >> 8;
		adress[1] = (adr >> 4) & 0x0F;
		adress[2] = adr & 0x0F;


		currSec->addSectionBinaryContent(0x80);
		currSec->addSectionBinaryContent(rgd << 4);
		currSec->addSectionBinaryContent((rgs << 4) | adress[0]);
		currSec->addSectionBinaryContent((adress[1] << 4) | adress[2]);

		pc += 4;
		break;
	}
	case REGSYMMEM:
	{
		cout << "Greska: simbol " << operand << "nije u odgovarajucem opsegu" << endl;
		break;
	}
	case ERROROP:
		break;
	default:
		break;
	}

}

void Assembler::secAndEndDirectiveSecondPass(string secName, TypeOfDirective dir)
{
	if (currSection.size() != 0) {
		SectionDefinition* curSec = sectionTable.at(currSection);
		if (curSec->getSizeOfLitPool() > 0) {
			for (int i = 0; i < curSec->getSizeOfLitPool(); i++) {
				int value = curSec->getLitValue(i);
				curSec->addSectionBinaryContent((char)value);
				value >>= 8;
				curSec->addSectionBinaryContent((char)value);
				value >>= 8;
				curSec->addSectionBinaryContent((char)value);
				value >>= 8;
				curSec->addSectionBinaryContent((char)value);
			}
		}
	}

	currSection = secName;
	if (secName.size()) currSectionNum = symbolTable.at(secName).getSecNum();
	pc = 0;
}

void Assembler::createELF()
{
	ofstream file(outputFile);

	if (!file.is_open()) {
		cout << "Greska! Dokument sa imenom " << outputFileName << " nije moguce otvoriti" << endl;
		exit(-1);
	}

	file << "#.symtab" << endl;
	file << setw(4) << setfill(' ') << "Num" << "\t"
		<< setw(8) << setfill(' ') << "Value" << "\t"
		<< setw(5) << setfill(' ') << "Type" << "\t"
		<< setw(5) << setfill(' ') << "Bind" << "\t"
		<< setw(4) << setfill(' ') << "Ndx"
		<< setw(16) << setfill(' ') << "Name" << endl;

	for (auto& entry : symbolTable) {
		SymbolDefinition sym = entry.second;
		string secID = (sym.getSymbolsSection() == "") ? "UND" : to_string(sym.getSecNum());
		file << setw(4) << setfill(' ') << sym.getSymID() << '\t'
			<< setw(8) << setfill('0') << setbase(16) << sym.getSymAdress() << '\t'
			<< setw(5) << setfill(' ') << ((sym.getSymType() == SECTION) ? "SCTN" : "NOTYP") << '\t'
			<< setw(5) << setfill(' ') << ((sym.getSymbolBinding() == LOCAL) ? "LOC" : "GLOB") << '\t'
			<< setw(4) << setfill(' ') << secID
			<< setw(16) << setfill(' ') << entry.first << endl;
	}

	for (auto& entry : sectionTable) {
		SectionDefinition* section = entry.second;
		file << "#." << entry.first << endl;
		list<char> content = section->getContent();
		int i = 0;
		for (char iter : content) {
			file << setw(2) << setfill('0') << setbase(16) << (iter & 0xFF);
			if (i % 4 == 3 && (i / 2) % 4 == 1)
			{
				file << "\t";
			}
			else if (i % 4 == 3)
			{
				file << endl;
			}
			else
			{
				file << " ";
			}
			i++;
		}
	}
	file << endl;
	for (auto& entry : sectionTable) {
		SectionDefinition* section = entry.second;
		file << "#.rel_" << entry.first << endl;
		file << setw(8) << setfill(' ') << "Offset" << "\t"
			<< setw(16) << setfill(' ') << "Type" << "\t"
			<< setw(4) << setfill(' ') << "Symbol" << "\t"
			<< setw(4) << setfill(' ') << "Addend" << endl;
		for (RelocationTableEntry rel : section->getRelTable())
		{
			if (rel.getRelSection() == entry.first)
			{
				file << setw(8) << setfill('0') << hex << rel.getRelOffset() << "\t"
					<< setw(16) << setfill(' ') << rel.getRelType() << "\t"
					<< setw(4) << setfill(' ') << (uint32_t)rel.getSymbolId() << "\t"
					<< setw(4) << setfill(' ') << (int)rel.getAddend() << endl;
			}
		}
	}

}

void Assembler::createBinary()
{
	ofstream binFile(this->inputFile.substr(0, inputFile.size() - 1) + "dat", ios::binary);

	if (!binFile.is_open()) {
		cout << "Greska! Dokument sa imenom " << outputFileName << " nije moguce otvoriti" << endl;
		exit(-1);
	}
	size_t symTableSize = symbolTable.size();
	binFile.write((char*)&symTableSize, sizeof(symTableSize));

	for (auto& entry : symbolTable) {
		SymbolDefinition sym = entry.second;
		string symName = entry.first, secName = sym.getSymbolsSection();
		size_t symNameSize = symName.size(), secNameSize = sym.getSymbolsSection().size();
		int symId = sym.getSymID(), secNum = sym.getSecNum();
		unsigned int symAdr = sym.getSymAdress();
		SymbolBinding bind = sym.getSymbolBinding();
		SymbolType type = sym.getSymType();
		bool isExt = sym.isSymbolExtern(), isDef = sym.isSymbolDefined();

		binFile.write((char*)&symNameSize, sizeof(symNameSize));
		binFile.write(symName.c_str(), symNameSize);


		binFile.write((char*)&symId, sizeof(symId));
		binFile.write((char*)&symAdr, sizeof(symAdr));
		binFile.write((char*)&secNum, sizeof(secNum));

		binFile.write((char*)&secNameSize, sizeof(secNameSize));
		binFile.write(secName.c_str(), secNameSize);
		binFile.write((char*)&bind, sizeof(bind));
		binFile.write((char*)&type, sizeof(type));
		binFile.write((char*)&isExt, sizeof(isExt));
		binFile.write((char*)&isDef, sizeof(isDef));
	}

	size_t sectionTableSize = sectionTable.size();
	binFile.write((char*)&sectionTableSize, sizeof(sectionTableSize));

	for (auto& entry : sectionTable) {
		SectionDefinition* section = entry.second;
		string sectionName = entry.first;
		size_t sectionNameSize = sectionName.size();
		int secId = section->getSectionID(), secSize = section->getSectionSize();

		binFile.write((char*)&sectionNameSize, sizeof(sectionNameSize));
		binFile.write(sectionName.c_str(), sectionNameSize);
		binFile.write((char*)&secId, sizeof(secId));
		binFile.write((char*)&secSize, sizeof(secSize));

		list<char> content = section->getContent();
		size_t contentSize = content.size();
		binFile.write((char*)&contentSize, sizeof(contentSize));
		for (char iter : content) {
			binFile.write((char*)&iter, sizeof(iter));
		}

		list<RelocationTableEntry> relTable = section->getRelTable();
		size_t relTableSize = relTable.size();
		binFile.write((char*)&relTableSize, sizeof(relTableSize));
		for (RelocationTableEntry iter : relTable) {
			size_t relTypeSize = iter.getRelType().size();
			unsigned int relOffset = iter.getRelOffset();
			string type = iter.getRelType();
			int symId = iter.getSymbolId(), addend = iter.getAddend();

			binFile.write((char*)&relOffset, sizeof(relOffset));
			binFile.write((char*)&relTypeSize, sizeof(relTypeSize));
			binFile.write(type.c_str(), relTypeSize);
			binFile.write((char*)&symId, sizeof(symId));
			binFile.write((char*)&addend, sizeof(addend));
		}
	}

}

char Assembler::regToChar(string reg)
{
	if (reg.find_first_of("0123456789") != string::npos) {
		return stoi((reg.substr(reg.find_first_of("0123456789"))));
	}
	else
	{
		if (reg == "sp") return 14;
		if (reg == "pc") return 15;
	}
	return 0;
}



#pragma warning( default : 4309 )
#pragma warning( default : 4312 )