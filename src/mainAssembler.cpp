#include "../inc/assembler.h"
#include <string>

using namespace std;


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Error! Too few arguments!" << endl;
		return -1;
	}

	string input;
	string output;
	string first = argv[1];

	if (first == "-o")
	{
		input = argv[3];
		output = argv[2];
	}
	else
	{
		input = argv[1];
		output = argv[1];
	}

	Assembler assembler(input, output);
	assembler.assemble();


/*	Assembler as2("handler.s", "handler.o");
	Assembler as1("math.s", "math.o");
	Assembler as("main.s", "main.o");
	Assembler as4("isr_terminal.s", "isr_terminal.o");
	Assembler as5("isr_timer.s", "isr_timer.o");
	Assembler as3("isr_software.s", "isr_software.o");

	as.assemble();
	as1.assemble();
	as2.assemble();
	as3.assemble();
	as4.assemble();
	as5.assemble();

	list<string> inputFiles;
	list<string> placements;

	inputFiles.push_back("handler.o");
	inputFiles.push_back("math.o");
	inputFiles.push_back("main.o");
	inputFiles.push_back("isr_terminal.o");
	inputFiles.push_back("isr_timer.o");
	inputFiles.push_back("isr_software.o");

	placements.push_back("-place=my_code@0x40000000");
	placements.push_back("-place=math@0xF0000000");

	Linker l(inputFiles, "linked.hex", placements);

	l.link();

	Emulator emul("reg.txt","linked.dat");

	emul.startEmulation();
*/
	
}

/*
./asembler -o handler.o handler.s 
./asembler -o main.o main.s 
./linker -hex  -place=my_code_main@0x40000000 -place=my_code_handler@0xC0000000 -o program.hex handler.o main.o 
./emulator program.hex
*/
