#include "../inc/assembler.h"
#include "../inc/linker/linker.h"
#include <list>
#include <string>

using namespace std;


int main()
{
	Assembler as2("handler.s", "handler.o");
	Assembler as1("math.s", "math.o");
	Assembler as("main.s", "main.o");
	Assembler as4("isr_terminal.s", "isr_terminal.o");
	Assembler as5("isr_timer.s", "isr_timer.o");
	Assembler as3("isr_software.s", "isr_software.o");

	as.assembler();
	as1.assembler();
	as2.assembler();
	as3.assembler();
	as4.assembler();
	as5.assembler();

	list<string> inputFiles;
	list<string> placements;

	inputFiles.push_back("handler.o");
	inputFiles.push_back("math.o");
	inputFiles.push_back("main.o");
	inputFiles.push_back("isr_terminal.o");
	inputFiles.push_back("isr_timer.o");
	inputFiles.push_back("isr_software.o");

	Linker l(inputFiles, "linked.hex", placements);

	l.link();
	
}