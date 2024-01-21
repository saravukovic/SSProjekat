#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <iomanip>

using namespace std;

class Emulator
{
public:
	Emulator(string output, string binary);
	~Emulator();

	void startEmulation();

private:

	void parseBinaryInput();
	uint32_t getBinaryContent(uint32_t source);
	uint32_t getInstruction(uint32_t source);
	void writeBinaryContent(uint32_t content, uint32_t dest);
	void emulate();
	void printReg();
	
	void executeInstr(uint32_t inst, uint32_t mod, uint32_t regA, uint32_t regB, uint32_t regC, int disp);

	map<uint32_t, char> memory;
	vector<uint32_t> GPReg, CSReg;
	uint32_t &pc = GPReg[15], &sp = GPReg[14];


	string outputFile, inputFile, binaryFile;
	bool emul;
};