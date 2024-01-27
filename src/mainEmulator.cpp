#include "../inc/emulator.h"
#include <list>
#include <string>

using namespace std;


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Error! Too few arguments!" << endl;
		return -1;
	}

	Emulator emulator(argv[1]);
	emulator.startEmulation();	
}