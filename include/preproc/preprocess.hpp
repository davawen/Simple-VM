#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <fstream>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <cstring>
#include <filesystem>

#include "error_handling.hpp"
#include "string_functions.hpp"
#include "preproc/line.hpp"
#include "preproc/macro.hpp"
#include "preproc/condition.hpp"

std::vector<Line> preprocess(const std::filesystem::path filename);
