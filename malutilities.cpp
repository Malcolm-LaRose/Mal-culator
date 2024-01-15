#include "malutilities.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


// File manipulation

std::string readFileIntoRawString(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return "";
    }

    std::ostringstream rawStringStream;
    rawStringStream << file.rdbuf();  // Read the entire file into the stream

    file.close();

    return rawStringStream.str();
}

void writeToTxtFile(const std::string& rawString, const std::string& fileName) {
    std::ofstream outputFile(fileName);

    if (outputFile.is_open()) {
        outputFile << rawString;
        outputFile.close();
        std::cout << "File '" << fileName << "' has been created successfully." << std::endl;
    }
    else {
        std::cerr << "Error creating file '" << fileName << "'." << std::endl;
    }
}

// Printing