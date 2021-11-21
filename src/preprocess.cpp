#include "preprocess.hpp"

/// Dumps the given file line by line to 'output'.
/// Returns false if there was an error when opening the file and true otherwise.
static bool read_file(const char *filename, std::vector<std::string> &output)
{
	std::ifstream file(filename);

	if(!file.is_open()) return false;

	std::string line;

	while(std::getline(file, line))
	{
		output.push_back(line);
	}

	return true;
}

/// Iterates over a string and calls a function for every characters in it, but ignores anything wrapped in double quotes
/// @param func Function used. Non zero return value indicates to break out of loop
static void iterate_ignore_quotes(std::string &str, size_t pos, int (*func)(std::string &, size_t &))
{
	bool inQuotes = false;

	while(pos < str.length())
	{
		if(inQuotes)
		{
			if(str[pos] == '\\')
			{
				pos++;
			}
			else if(str[pos] == '\"')
			{
				inQuotes = false;
			}
		}
		else if(str[pos] == '"')
		{
			inQuotes = true;
		}
		else
		{
			int res = func(str, pos);

			if(res != 0) break;
		}

		pos++;
	} 
}

static void iterate_ignore_quotes(std::string &str, int (*func)(std::string &, size_t &))
{
	iterate_ignore_quotes(str, 0, func);
}

/// Search string 'str' for the string 'query', which is either have the the given delimeters on its side, or be on the borders of the string 
/// Returns the index where query starts if it was found, and std::string::npos otherwise
static size_t find_string(const std::string &str, const std::string &query, const char *delimeters)
{
	size_t i, j = 0;
	for(i = 0; i < str.length(); i++)
	{
		if(str[i] == query[j])
		{
			size_t startValue = i;

			// TODO: Replace this to use std::string::find instead

			while(str[i] == query[j])
			{
				i++;
				j++;

				if(i >= str.length()) break;
				if(j >= query.length()) break;
			}

			if(j < query.length()) // Couldn't finish searching for string
			{
				i = startValue;
				j = 0;
			}
			else if(i >= str.length() || strchr(delimeters, str[i]))
			{
				return startValue;
			}
		}
	}

	return std::string::npos;
}

void preprocess(const char *filename, std::vector<std::string> &output)
{
	if(!read_file(filename, output))
	{
		compile_error(0, "Source file does not exists: %s", filename); // This is checked in main but oh well
	}
	
	std::string filenameDirectory = filename;
	filenameDirectory.erase(filenameDirectory.find_last_of('/') + 1);
	
	// Include directives
	size_t lineNum = 1;
	auto it = output.begin();
	std::unordered_set<std::string> includedFiles;

	// TODO: Heriarchical view of inclusion for error logging
	// Or some kind if way to see who included what where
	
	while(it != output.end())
	{
		std::string &line = *it;

		if(line[0] == '#')
		{
			// Chech for preprocessor directives

			if(line.find("#include") != std::string::npos)
			{
				// TODO: Make marking files more robust
				std::string includeFile = line.substr(line.find("\"") + 1, line.rfind("\"") - line.find("\"") - 1);
				if(line.find("#include_recursive") == std::string::npos && includedFiles.find(includeFile) != includedFiles.end()) // If file already included and not recursive
				{
					// Already included
					it = output.erase(it);
					continue;
				}

				includedFiles.insert(includeFile);

				includeFile = filenameDirectory + includeFile;

				std::vector<std::string> include_output;

				if(!read_file(includeFile.c_str(), include_output))
				{
					// Line num isn't accurate, but it works for now
					compile_error(lineNum, "Included file does not exists: %s", includeFile.c_str());
				}

				auto newIt = output.insert(it, include_output.begin(), include_output.end());
				output.erase(it);
				it = newIt;
				it--; // Counter-balance it++
			}
		}

		it++;
		lineNum++;
	}
	
	// Remove trailing spaces / comments
	lineNum = 1;
	it = output.begin();
	while(it != output.end())
	{
		std::string &line = *it;
		
		// Remove comments
		iterate_ignore_quotes(line,
			[](std::string &line, size_t &i)
			{
				if(line[i] == ';')
				{
					line.erase(i);
					return 1;
				}

				return 0;
			}
		);

		// Remove blanks at the beginning
		size_t i = 0;
		while(true)
		{
			if(!isblank(line[i])) break;

			i++;
		}

		if(i > 0)
		{
			line.erase(0, i);
		}

		// Remove blanks between arguments
		iterate_ignore_quotes(line,
			[](std::string &line, size_t &i)
			{
				if(isblank(line[i]) && (line[i-1] == ',' || isblank(line[i-1])))
				{
					line.erase(i, 1);
					i--;
				}

				return 0;
			}
		);

		// Remove all blanks at the end
		// Useful for instructions with no arguments
		i = line.length() - 1;
		while(i > 0 && i < line.length()) // Avoid integer overflow if length is 0
		{
			if(!isblank(line[i])) break;

			i--;
		}

		
		if(i > 0 && i < line.length() - 1)
		{
			line.erase(i + 1);
		}

		it++;
		lineNum++;
	}

	// Process macros definitions
	std::unordered_map<std::string, std::vector<std::string>> macros;
	
	it = output.begin();
	while(it != output.end())
	{
		std::string &line = *it;

		size_t pos;
		if((pos = line.find("#define")) != std::string::npos)
		{
			pos += sizeof("#define"); // size of "#define" *with* NUL character, which counts as the space

			// #define MACRO VALUE
			//         ^pos ^posValue

			size_t posValue = line.find(' ', pos);

			std::string name = line.substr(pos, posValue - pos); // variable set for readability, should get optimized away

			macros[name].push_back(line.substr(posValue + 1));

			// std::cout << "Defined: \"" << name << "\"\n";

			it = --output.erase(it);
		}
		else if((pos = line.find("#macro ")) != std::string::npos) // NOTE: Space is required to not check against #macrogroup
		{
			pos += sizeof("#macro");
			
			std::string name = line.substr(pos);

			//#macro MACRO
			//  ..
			//#endmacro

			auto startOfMacro = it;

			while(true)
			{
				it++;
				line = *it;

				if(line[0] == '#' && find_string(line, "#endmacro", "") != std::string::npos) break;

				macros[name].push_back(line);
			}

			if(macros[name].size() == 0)
			{
				compile_error(0, "Empty macro %s", name.c_str()); // TODO: Line numbers soon to be added
			}

			// std::cout << "Defined: \"" << name << "\n";

			it = output.erase(startOfMacro, it + 1) - 1;
		}
		else
		{
			for(auto &pair : macros)
			{
				size_t index;
				if((index = find_string(line, pair.first, " ,\t")) != std::string::npos)
				{
					line.erase(index, pair.first.length());
				}
				else if((index = line.find("#(" + pair.first + ")")) != std::string::npos)
				{
					line.erase(index, pair.first.length() + sizeof("#()") - 1);
				}

				if(index != std::string::npos)
				{
					line.insert(index, pair.second[0]);

					if(pair.second.size() > 1) output.insert(it + 1, pair.second.begin() + 1, pair.second.end());

					it--;
				}
			}
		}

		it++;
	}

	/*for(auto &pair: macros)
	{
		std::cout << "Macro: \"" << pair.first << "\"\n{\n";
		for(auto &str : pair.second)
		{
			std::cout << "\t" << str << "\n";
		}
		std::cout << "}\n";
	}*/

	// Process macro groups

	// Process other directives
}
