#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <exception>
#include <ostream>
#include <algorithm>

#include <cstring>
#include <ctime>
#include <cinttypes>

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
	
	parse_instructions(argv[1], instructions);
	

	printf("\n");
	std::for_each(instructions.begin(), instructions.end(), [](Instruction &ins){ std::cout << ins << '\n'; });
	
	//return 0;

	printf("\n[STARTING EXECUTION FROM HERE]\n\n");

	constexpr int STACK_SIZE = 10000;
	int stack[STACK_SIZE];
	int *sp = stack;
	
	// Register is apparently a reserved c++ keyword *shrugs*
	int reg = 0;

	auto fatalErr = [](const char *str) -> void
	{
		printf("\x1b[41mFatal error: %s.\n", str);
		throw std::runtime_error(str);
	};

	auto warningErr = [](const char *str) -> void
	{
		printf("\x1b[43mWarning: %s.\n", str);
	};


	for(auto iterator = instructions.begin(); iterator != instructions.end(); iterator++)
	{
		auto &it = *iterator;

		// Error checking
		if(sp < stack)
		{
			// TODO: Add flavor text (... at line xx and instruction xx)
			fatalErr("Stack underflow");
		}

		if(sp > stack + STACK_SIZE)
		{
			warningErr("Stack overflow, probable segmentation fault");
		}
		// std::cout << it << '\n';
		
		switch(it.type)
		{
			case Instruction::Type::PUSH:
				if(it.numValues == 0)
				{
					// Push register value instead
					sp++;
					*sp = reg;
				}
				for(int i = 0; i < it.numValues; i++)
				{
					sp++;	
					*sp = it.value[i].val;
				}
				break;
			case Instruction::Type::POP:
				// Pop current stack value to register
				reg = *sp;
				sp--;
				break;
			case Instruction::Type::PRINT:
				for(int i = 0; i < it.numValues; i++)
				{
					// Will add support for more types later
					if(it.value[i].type == Value::Type::STRING)
						printf("%s", reinterpret_cast<char *>(it.value[i].val));
					else
					 	printf("%" PRId64, it.value[i].val);
				}
				break;
			case Instruction::Type::IFEQ: // Only evaluates next instruction if condition is true
				// ifeq [comparator],[value]
				
				break;
			case Instruction::Type::JUMP:
			{
				// Find matching label with a linear search
				// Efficicency is shit
				// Labels should be stored in their own vector
				
				auto match = instructions.end();
				for(auto itlabel = instructions.begin(); itlabel != instructions.end(); itlabel++)
				{
					if(itlabel->type != Instruction::Type::LABEL) continue;

					if(itlabel->value->val == it.value->val) match = itlabel;
				}

				if(match == instructions.end())
				{
					warningErr("Label does not exists."); // This should be checked at "compile" time
					break;
				}
				
				iterator = match;
				break;
			}
			default:
				break;
		}
	}
	
	
	
	
	
	
	return 0;
}
