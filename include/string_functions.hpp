#pragma once

#include <string>
#include <optional>
#include <algorithm>
#include <stdexcept>

#include <cstring>

/// Generates a random alpha-numeric string of the given length
std::string random_string(size_t length);

/// Takes a slice from a string, from index a to index b
std::string string_slice(const std::string &self, size_t a, size_t b);

/// Search string 'str' for the string 'query', which is either have the the given delimeters on its side, or be on the borders of the string 
/// @returns the index where query starts if it was found, and std::string::npos otherwise
size_t find_delimited_string(const std::string &str, const std::string &query, const char *delimeters);


/// Iterates over a string and calls a function for every characters in it, but ignores anything wrapped in double quotes
/// @param func function used. Non zero return value indicates to break out of loop
void iterate_ignore_quotes(std::string &str, size_t pos, int (*func)(std::string &, size_t &));
void iterate_ignore_quotes(std::string &str, int (*func)(std::string &, size_t &));


/// Finds a string inside another, ignoring anything inside double quotes
/// @param self string which is searched
/// @param target string to search
/// @param pos position from which to start the search
/// @returns Start index of target string, or std::string::npos if it isn't found
size_t find_ignore_quotes(const std::string &self, const std::string &target, size_t pos = 0);

/// Finds a character inside a given string, ignoring anything inside double quotes
/// @param self string inside which to search
/// @param target Character to search
/// @param pos Position from which to start the search
/// @returns Start index of target character, or std::string::npos if it isn't found
size_t find_ignore_quotes(const std::string &self, char target, size_t pos = 0);
