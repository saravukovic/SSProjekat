#include "../inc/linker/linker.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Error! Too few arguments!" << endl;
		return -1;
	}

	list<string> input;
	string output;
	list<string> placement;
  string argument;
  bool linkingEnabled = false;

  for(int i = 1; i < argc; i++)
  {
    argument = argv[i];
    if (argument == "-o")
    {
      if (output != "")
      {
        cout << "Error! Only one output file can be defined!" << endl;
		    return -1;
      }
      else{
        i++;
        argument = argv[i];
        if(argument.find(".hex") == string::npos)
        {
          cout << "Error! Only .hex files are supported as output file for this linker!" << endl;
		      return -1;
        }
        output = argument;
      }
    }
    else if(argument.find("place=") != string::npos)
    {
      if(argument.find("@") != string::npos)
      {
        placement.push_back(argument);
      }
      else
      {
        cout << "Error! Placement option is not in right format!" << endl;
		    return -1;
      }
    }
    else if (argument == "-hex")
    {
      linkingEnabled = true;
    }
    else if(argument.find(".o") != string::npos)
    {
      if(linkingEnabled)
      {
        input.push_back(argument);
      }
    }
  }

  if(!linkingEnabled)
  {
    cout << "Error! Missing command for linking." << endl;
		return -1;
  }

	if (output == "")
	{
		output = "linked.hex";
	}

	Linker linker(input, output, placement);
	linker.link();
}