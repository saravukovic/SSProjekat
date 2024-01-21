#include "..\inc\emulator.h"
#include "..\inc\syntaxCheck.h"

Emulator::Emulator(string output, string binary) : GPReg(16, 0), CSReg(3, 0)
{
	string path = "C:\\Users\\Sara\\Desktop\\ssproba\\";
	outputFile = path + output;
	binaryFile = path + binary;
	emul = false;
	pc = 0x40000000;
	
}

Emulator::~Emulator()
{
}

void Emulator::startEmulation()
{
	parseBinaryInput();
	emul = true;
	emulate();
	printReg();
}

void Emulator::parseBinaryInput()
{

	ifstream file(binaryFile, ios::binary);

	if (!file.is_open()) {
		cout << "Greska! Dokument sa imenom " << binaryFile << " nije moguce otvoriti" << endl;
		exit(-1);
	}

	size_t secTableSize = 0;
	file.read((char*)&secTableSize, sizeof(secTableSize));

	while (secTableSize) {
		unsigned int adress = 0;
		size_t contentSize = 0;

		file.read((char*)&adress, sizeof(adress));
		file.read((char*)&contentSize, sizeof(contentSize));

		int i = 0;

		while (i < contentSize) {
			char c = 0;
			file.read((char*)&c, sizeof(c));
			memory.insert({ adress + i, c });
			i++;
		}

		secTableSize--;
	}

}

uint32_t Emulator::getBinaryContent(uint32_t source)
{
	uint32_t val = 0;
	for (int i = 0; i < 4; i++) {
		uint32_t c = ((uint32_t)memory[source + i] & 0xFF);
		val |= ((uint32_t)(c << i * 8));
	}
	return val;
}

uint32_t Emulator::getInstruction(uint32_t source)
{
	uint32_t val = 0;
	for (int i = 0; i < 4; i++) {
		uint32_t c = ((uint32_t)memory.at(source + i) & 0xFF);
		val |= ((uint32_t)(c << ((3 - i) * 8)));
	}
	return val;
}

void Emulator::writeBinaryContent(uint32_t content, uint32_t dest)
{
	for (int i = 0; i < 4; i++) {
		memory[dest + i] = (char)(content >> (i * 8));
	}
}

void Emulator::emulate()
{
	uint32_t wholeInstr = 0x00000000;
	uint32_t regA, regB, regC, disp, mod, instr;
	while (emul) {
		wholeInstr = 0x00000000;
		wholeInstr = getInstruction(pc);
		pc += 4;
		disp = wholeInstr & 0x00000FFF;
		if ((disp >> 2 * 4) & 8) disp |= 0xFFFFF000;
		regC = (wholeInstr >> (3 * 4)) & 0x0000000F;
		regB = (wholeInstr >> (4 * 4)) & 0x0000000F;
		regA = (wholeInstr >> (5 * 4)) & 0x0000000F;
		mod = (wholeInstr >> (6 * 4)) & 0x0000000F;
		instr = (wholeInstr >> (7 * 4)) & 0x0000000F;
		executeInstr(instr, mod, regA, regB, regC, disp);
	}
}

void Emulator::printReg()
{
	ofstream file(outputFile, ios::binary);
	for (int i = 0; i < 14; i++) {
		file << "r" << i << " = ";
		file << setw(8) << setfill('0') << hex << GPReg[i] << "\t";
		if (i % 4 == 3) file << endl;
	}
	file << "r" << 14 << " = ";
	file << setw(8) << setfill('0') << hex << sp << "\t";
	file << "r" << 15 << " = ";
	file << setw(8) << setfill('0') << hex << pc << "\t";
}

void Emulator::executeInstr(uint32_t inst, uint32_t mod, uint32_t regA, uint32_t regB, uint32_t regC, int disp)
{
	//reg0 na nulu

	switch (inst)
	{
	case 0x0:
		emul = false;
		break;
	case 0x1:
		sp -= 4;
		writeBinaryContent(CSReg[1], sp);
		sp -= 4;
		writeBinaryContent(pc, sp);
		CSReg[2] = 4;
		CSReg[0] = (CSReg[0] & (~0x1));
		pc = CSReg[1];
		break;
	case 0x2:
		switch (mod)
		{
		case 0x0:
			sp -= 4;
			writeBinaryContent(pc, sp);
			pc = GPReg[regA] + GPReg[regB] + disp;
			break;
		case 0x1:
			sp -= 4;
			writeBinaryContent(pc, sp);
			pc = getBinaryContent(GPReg[regA] + GPReg[regB] + disp);
			break;
		default:
			break;
		}
		break;
	case 0x3:
		switch (mod)
		{
		case 0x0:
			pc = GPReg[regA] + disp;
			break;
		case 0x1:
			if (GPReg[regB] == GPReg[regC]) {
				pc = GPReg[regA] + disp;
			}
			break;
		case 0x2:
			if (GPReg[regB] != GPReg[regC]) {
				pc = GPReg[regA] + disp;
			}
			break;
		case 0x3:
			if ((int)GPReg[regB] > (int)GPReg[regC]) {
				pc = GPReg[regA] + disp;
			}
			break;
		case 0x8:
			pc = getBinaryContent(GPReg[regA] + disp);
			break;
		case 0x9:
			if (GPReg[regB] == GPReg[regC]) {
				pc = getBinaryContent(GPReg[regA] + disp);
			}
			break;
		case 0xA:
			if (GPReg[regB] != GPReg[regC]) {
				pc = getBinaryContent(GPReg[regA] + disp);
			}
			break;
		case 0xB:
			if ((int)GPReg[regB] > (int)GPReg[regC]) {
				pc = getBinaryContent(GPReg[regA] + disp);
			}
			break;
		default:
			break;
		}
		break;
	case 0x4:
	{
		uint32_t temp = GPReg[regB];
		GPReg[regB] = GPReg[regC];
		GPReg[regC] = temp;
	}
		break;
	case 0x5:
		switch (mod)
		{
		case 0x0:
			GPReg[regA] = GPReg[regB] + GPReg[regC];
			break;
		case 0x1:
			GPReg[regA] = GPReg[regB] - GPReg[regC];
			break;
		case 0x2:
			GPReg[regA] = GPReg[regB] * GPReg[regC];
			break;
		case 0x3:
			GPReg[regA] = GPReg[regB] / GPReg[regC];
			break;
		default:
			break;
		}
		break;
	case 0x6:
		switch (mod)
		{
		case 0x0:
			GPReg[regA] = ~GPReg[regB];
			break;
		case 0x1:
			GPReg[regA] = GPReg[regB] & GPReg[regC];
			break;
		case 0x2:
			GPReg[regA] = GPReg[regB] | GPReg[regC];
			break;
		case 0x3:
			GPReg[regA] = GPReg[regB] ^ GPReg[regC];
			break;
		default:
			break;
		}
		break;
	case 0x7:
		switch (mod)
		{
		case 0x0:
			GPReg[regA] = GPReg[regB] << GPReg[regC];
			break;
		case 0x1:
			GPReg[regA] = GPReg[regB] >> GPReg[regC];
			break;
		default:
			break;
		}
		break;
	case 0x8:
		switch (mod)
		{
		case 0x0:
			writeBinaryContent(GPReg[regC], GPReg[regA] + GPReg[regB] + disp);
			break;
		case 0x1:
			GPReg[regA] = GPReg[regA] + disp;
			writeBinaryContent(GPReg[regC], GPReg[regA]);
			break;
		case 0x2:
			writeBinaryContent(GPReg[regC], getBinaryContent(GPReg[regA] + GPReg[regB] + disp));
			break;
		default:
			break;
		}
		break;
	case 0x9:
		switch (mod)
		{
		case 0x0:
			GPReg[regA] = CSReg[regB];
			break;
		case 0x1:
			GPReg[regA] = GPReg[regB] + disp;
			break;
		case 0x2:
			GPReg[regA] = getBinaryContent(GPReg[regB] + GPReg[regC] + disp);
			break;
		case 0x3:
			GPReg[regA] = getBinaryContent(GPReg[regB]);
			GPReg[regB] = GPReg[regB] + disp;
			break;
		case 0x4:
			CSReg[regA] = GPReg[regB];
			break;
		case 0x5:
			CSReg[regA] = CSReg[regB] | disp;
			break;
		case 0x6:
			CSReg[regA] = getBinaryContent(GPReg[regB] + GPReg[regC] + disp);
			break;
		case 0x7:
			CSReg[regA] = getBinaryContent(GPReg[regB]);
			GPReg[regB] = GPReg[regB] + disp;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	GPReg[0] = 0;
}
