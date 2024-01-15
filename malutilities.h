#pragma once

#ifndef MALUTILITIES
#define MALUTILITIES

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

std::string readFileIntoRawString(const std::string& fileName);
void writeToTxtFile(const std::string& rawString, const std::string& fileName);

#endif