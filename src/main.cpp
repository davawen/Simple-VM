#include <iostream>
#include <vector>
#include <exception>
#include <ostream>

#include <cstring>
#include <ctime>

#include "instruction.hpp"
#include "parseInstruction.hpp"

#define M_LOOP(x) for(int idx__ = 0; idx__ < x; idx__++)

// int str_cut(char *str, int begin, int len)
// {
//     int l = strlen(str);

//     if (len < 0) len = l - begin;
//     if (begin + len > l) len = l - begin;
//     memmove(str + begin, str + begin + len, l - len + 1);

//     return len;
// }

int main(int argc, char *argv[])
{
	if(argc <= 1)
	{
		printf("Error: No file specified.\n");
		exit(-1);
	}
	
	
	// Parse file
	std::vector<Instruction> instructions;
	
	parseInstructions(argv[1], instructions);
	
	for(auto &it : instructions)
	{
		std::cout << it << '\n';
	}
	
	// int stack[10000];
	// int *sp = stack;
	
	
	
	return 0;
}