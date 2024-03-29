/*

    ABANDON HOPE, ALL YE WHO ENTER



    Written by Malcolm LaRose for C++/17 in February, the year of our Lord, 2024, the 6th day.
    Free to use and edit, but please attribute me if you publish anything based on this

TODO:

    Finish getResults and getEnergy functions
    Move to utilities file to be used by factoriocalc
    Figure out how to extract data
    Figure out how to put data into Recipes
        struct?

    Bonus:
    Convert factorio recipe.lua to TextFile1.txt
    Clean up intermediate steps (refactor)
        * Refactor part that looks for stuff within brackets that may contain brackets themselves
    Optimization

*/

#include "malutilities.h"

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cctype>  // Include the header for std::isspace


// FUNCTION PROTOTYPES
std::string extractNestedBraces(const std::string& input);
std::vector<std::string> extractIndividualMembers(const std::string& input);
std::string getNormal(const std::string& inputMember);
void removeLeadingWhitespace(std::string& text);
void removeLineStartingWith(std::string& text, const std::string& sequence);
std::string removeQuotes(const std::string& input);
void ensureNewlineAfterBrace(std::string& text);
void insertNewlineBetweenBrackets(std::string& input);
void removeNewlineAfterEquals(std::string& text);
std::string removeCraftingMachineTint(std::string& inputMember);
void checkForLastComma(std::vector<std::string> inputMember, int i);
bool checkForLastCommaBool(std::vector<std::string> inputMember, int i);
std::string parseIngredients(const std::string& ingredientsData);
std::string getIngredients(std::string& inputMember);
std::string getResults(std::string& inputMember);
std::string getRecipeName(std::string& inputMember);
std::string getEnergy(std::string& inputMember);



// Works on one member (or category) to extract contents from within nested braces
std::string extractNestedBraces(const std::string& input) {
    size_t depth = 0;
    size_t start = std::string::npos;
    size_t end = std::string::npos;

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '{') {
            if (depth == 0) {
                start = i;
            }
            ++depth;
        }
        else if (input[i] == '}') {
            --depth;
            if (depth == 0) {
                end = i;
            }
        }

        if (depth == 0 && start != std::string::npos && end != std::string::npos) {
            return input.substr(start, end - start + 1);
        }
    }

    return "";  // No match found
}

// Parses members from the rawString from the .txt recipes file
std::vector<std::string> extractIndividualMembers(const std::string& input) {
    std::vector<std::string> members;
    size_t pos = 0;

    int numMems{ 0 };

    while (true) {
        size_t start = input.find_first_of('{', pos);
        if (start == std::string::npos) {
            break;  // No more opening braces found
        }

        size_t end = start;
        int depth = 1;

        while (depth > 0 && ++end < input.length()) {
            if (input[end] == '{') {
                ++depth;
            }
            else if (input[end] == '}') {
                --depth;
            }
        }

        if (depth == 0) {
            members.push_back(input.substr(start, end - start + 1));
            pos = end + 1;  // Move the position to the character after the closing brace
        }
        else {
            break;  // Invalid nested braces
        }
        numMems += 1;
    }

    // std::cout << "HERE: " << numMems << std::endl;

    return members;
}


// Gets normal recipe
std::string getNormal(const std::string& inputMember) {
    // Find the position of "expensive" and "normal"
    size_t expensivePos = inputMember.find("expensive");
    size_t normalPos = inputMember.find("normal");

    // If both "expensive" and "normal" are found, remove them and return the modified member
    if (expensivePos != std::string::npos && normalPos != std::string::npos) {
        // Extract the contents of "normal" excluding the first and last lines
        std::string normalContents = extractNestedBraces(inputMember.substr(normalPos));

        // Find the positions of the first and last newlines in normalContents
        size_t firstNewline = normalContents.find_first_of('\n');
        size_t lastNewline = normalContents.find_last_of('\n');

        // If both newlines are found, remove the first and last lines
        if (firstNewline != std::string::npos && lastNewline != std::string::npos && firstNewline < lastNewline) {
            normalContents = normalContents.substr(firstNewline + 1, lastNewline - firstNewline - 1);
        }

        // Remove "expensive" and "normal" parts and the extra set of braces
        std::string modifiedMember = inputMember.substr(0, normalPos - 1) + normalContents;

        return modifiedMember;
    }

    // If "expensive" or "normal" is not found, return the original member
    return inputMember;
}

// Functions to use on rawstring or transformed string

// Clean up rawString
void removeLeadingWhitespace(std::string& text) {
    size_t pos = 0;

    while (pos < text.length()) {
        // Find the first non-whitespace character on the current line
        size_t lineStart = pos;
        while (pos < text.length() && std::isspace(text[pos]) && text[pos] != '\n') {
            ++pos;
        }

        // Erase leading whitespace on the current line
        text.erase(lineStart, pos - lineStart);

        // Move to the next line
        while (pos < text.length() && text[pos] != '\n') {
            ++pos;
        }
        ++pos;  // Move past the newline character
    }
}

// Remove simple one liner categories
void removeLineStartingWith(std::string& text, const std::string& sequence) {
    // Find the first occurrence of the sequence in the text
    size_t pos = text.find(sequence);

    while (pos != std::string::npos) {
        // Check if the sequence is at the beginning or has whitespace before it
        if (pos == 0 || std::isspace(text[pos - 1])) {
            // Find the end of the line
            size_t end = text.find('\n', pos);

            // Erase the entire line
            text.erase(pos, end - pos + 1);
        }

        // Continue searching for the next occurrence
        pos = text.find(sequence, pos + 1);
    }
}

// Function to remove every instance of the character '"' from a string
std::string removeQuotes(const std::string& input) {
    std::string result;

    for (char ch : input) {
        // Filter out the '"' character
        if (ch != '"') {
            result += ch;
        }
    }

    return result;
}


void ensureNewlineAfterBrace(std::string& text) {
    size_t pos = 0;

    while (pos < text.length()) {

        // Find the next '},' in the current line
        size_t bracePos = text.find("},", pos);

        if (bracePos != std::string::npos) {
            // Check if there is no newline immediately after the '},'
            if (bracePos + 2 >= text.length() || text[bracePos + 2] != '\n') {
                // Insert a newline character after the '},'
                text.insert(bracePos + 2, "\n");
            }
        }

        // Move to the next line
        while (pos < text.length() && text[pos] != '\n') {
            ++pos;
        }
        ++pos;  // Move past the newline character
    }
}

void insertNewlineBetweenBrackets(std::string& input) {
    // Iterate through the string
    for (size_t i = 0; i < input.size() - 1; ++i) {
        // Check for consecutive opening brackets
        if (input[i] == '{' && input[i + 1] == '{') {
            // Insert a newline after the first bracket
            input.insert(i + 1, "\n");
            // Increment the loop index to skip the newly inserted newline
            ++i;
        }
        // Check for consecutive closing brackets
        else if (input[i] == '}' && input[i + 1] == '}') {
            // Insert a newline after the first bracket
            input.insert(i + 1, "\n");
            // Increment the loop index to skip the newly inserted newline
            ++i;
        }
    }
}


void removeNewlineAfterEquals(std::string& text) {
    size_t pos = 0;

    // Find the first occurrence of '=' in the text
    size_t equalsPos = text.find('=');

    while (equalsPos != std::string::npos) {
        // Find the first newline character after the '='
        size_t newlinePos = text.find('\n', equalsPos);

        // If newline is found and there are only whitespace characters in between, remove it
        if (newlinePos != std::string::npos) {
            size_t i = equalsPos + 1;
            while (i < newlinePos && std::isspace(text[i])) {
                ++i;
            }
            if (i == newlinePos) {
                text.erase(newlinePos, 1);
                text.insert(i, " ");
            }
        }

        // Move to the next '='
        equalsPos = text.find('=', equalsPos + 1);
    }
}



std::string removeCraftingMachineTint(std::string& inputMember) {
    // Find the position of "crafting_machine_tint"
    size_t CMTPos = inputMember.find("crafting_machine_tint");

    // If crafting_machine_tint found, remove and return the modified member
    if (CMTPos != std::string::npos) {
        // Extract the contents of "normal" excluding the first and last lines
        std::string CMTContents = extractNestedBraces(inputMember.substr(CMTPos));

        // Find the positions of the first and last newlines in normalContents
        size_t firstNewline = CMTContents.find_first_of('\n');
        size_t lastNewline = CMTContents.find_last_of('\n');

        // If both newlines are found, remove the first and last lines
        if (firstNewline != std::string::npos && lastNewline != std::string::npos && firstNewline < lastNewline) {
            CMTContents = CMTContents.substr(firstNewline + 1, lastNewline - firstNewline - 1);
        }

        // Remove crafting_machine_tint category and the extra set of braces
        std::string modifiedMember = inputMember.substr(0, CMTPos - 1);

        return modifiedMember;
    }

    // If crafting_machine_tint is not found, return the original member (do nothing)
    return inputMember;
}


void checkForLastComma(std::vector<std::string> inputMember, int i) {
    if (i < inputMember.size() - 1) {
        std::cout << ',';
    }

}

bool checkForLastCommaBool(std::vector<std::string> inputMember, int i) {
    if (i < inputMember.size() - 1) {
        return true;
    }
    return false;
}


// Function to parse ingredients data and output in the format {name, amount}
std::string parseIngredients(const std::string& ingredientsData) {
    std::vector<std::string> result;
    size_t pos = 0;

    // Find and parse each occurrence of "name=" and "amount="
    while ((pos = ingredientsData.find("name=", pos)) != std::string::npos) {
        // Find the end of the current "name="
        size_t endPos = ingredientsData.find(',', pos);
        if (endPos == std::string::npos) {
            break; // If ',' not found, exit loop
        }

        // Extract the name substring
        std::string name = ingredientsData.substr(pos + 6, endPos - pos - 7);

        // Find the corresponding "amount=" after the name
        size_t amountPos = ingredientsData.find("amount=", endPos);
        if (amountPos == std::string::npos) {
            break; // If "amount=" not found, exit loop
        }

        // Find the end of the current "amount="
        size_t amountEndPos = ingredientsData.find('}', amountPos);
        if (amountEndPos == std::string::npos) {
            break; // If '}' not found, exit loop
        }

        // Extract the amount substring and convert it to an integer
        int amount = std::stoi(ingredientsData.substr(amountPos + 7, amountEndPos - amountPos - 7));

        // Format the output string as {name, amount} and add it to the result vector
        result.push_back("{" + name + ", " + std::to_string(amount) + "}");

        // Move the position forward
        pos = amountEndPos;
    }

    // Convert the vector to a single string
    std::string output;
    for (const auto& ingredient : result) {
        output += ingredient + '\n';
    }

    std::string finalOut = "{ \n" + output + "}";

    return finalOut;
}

std::string getIngredients(std::string& inputMember) {
    // Position of ingredients string
    size_t ingPos = inputMember.find("ingredients");
    // std::cout << "ingPos: " << ingPos << std::endl;

    if (ingPos != std::string::npos) {
        // Extract the contents of "ingredients" including the braces
        std::string ingContents = extractNestedBraces(inputMember.substr(ingPos));

        //std::cout << "ING CONTS: " << ingContents << std::endl;


        // If the contents are weird (have ids like "name" or "type") handle that here 
        size_t typePos = ingContents.find("type=");

        std::string outputPair("");

        if (typePos == std::string::npos) { // Ingredients aren't weird, handle normally
            outputPair = ingContents;
        }

        else if (typePos != std::string::npos) { // Ingredients are weird (fluid recipe)
            outputPair = parseIngredients(ingContents);
        }

        else { // Fallthrough
            outputPair = ingContents;
        }

        ensureNewlineAfterBrace(outputPair); // Cleans up any weird braces
        insertNewlineBetweenBrackets(outputPair); // And the same

        return outputPair;
    }
    else { 
        // If "ingredients" is not found, panic
        return "panic";
    }

}


std::string getResults(std::string& inputMember) {

    // Check for results and result count
    // // If result to one thing
    // // If results do another thing
    // // If no result count, result count = 1

    size_t resultPos = inputMember.find("result");
    size_t resultsPos = inputMember.find("results");

    std::string rOutput = "";

    if (resultsPos != std::string::npos) {
        // There are results
        std::string resultsContents = extractNestedBraces(inputMember.substr(resultsPos));
        rOutput = resultsContents;
    }

    else if ((resultPos != std::string::npos) and (resultsPos == std::string::npos)) {
        // There is a result
        std::string resultContents = extractNestedBraces(inputMember.substr(resultPos));
        rOutput = resultContents;

    }

    else {
        // PANICK
        std::cout << "PANICKING!" << std::endl;
    }

    return rOutput;
}

std::string getRecipeName(std::string& inputMember) {

    std::string result;
    size_t pos = inputMember.find("name = ");

    // Find the end of the current "name = " 8 chars
    size_t endPos = inputMember.find(',', pos); // First comma after name = 
    if (endPos == std::string::npos) { // Name not found
        result = "PaNiC";
    }

    // Extract the name substring
    std::string name = inputMember.substr(pos + 8, endPos - pos - 9);


    // Format the output string as {name, amount} and add it to the result vector
    result = name;

    return name;
}

std::string getEnergy(std::string& inputMember) {

    // Check for energy required
    // If no energy required, energy required = 1

    std::string result;
    size_t pos = inputMember.find("energy_required ="); // 18 chars

    size_t endPos = inputMember.find(',', pos);

    if (endPos == std::string::npos) {
        result = "1";
    }
    else if (endPos != std::string::npos) {
        result = inputMember.substr(pos + 18, endPos - pos - 19);
    }
    else {
        result = "penic";
    }



    return result;
}



int main() {
    std::string filename = "TextFile1.txt";
    std::string rawString = readFileIntoRawString(filename);

    // Either grab the required info or trim the unrequired info
    // Notes:
        // Trim expensive recipes from normal ones
        // Need name, ingredients, results, result count, and energy required
        // If there is no energy required, assume 1
        // If there is no result count, assume 1
        // Look for results first, use result if it doesn't exist
    // Categories to trim: type, category, enabled, icon, icon_size, icon_mipmaps, subgroup, order, allow_decomposition, main_product, crafting_machine_tint,



    // Functions that act on the whole raw string
    // ONE LINE CATEGORIES ONLY!!!
    std::vector<std::string> catsToRemove = { "type", "category", "enabled", "order", "allow_decomposition", "main_product", "subgroup", "requester_paste_multiplier"};

    removeLeadingWhitespace(rawString);
    ensureNewlineAfterBrace(rawString);
    removeLeadingWhitespace(rawString);

    for (auto& vi : catsToRemove) {
        removeLineStartingWith(rawString, vi);
    }
    removeLineStartingWith(rawString, "icon"); // Not perfect

    removeNewlineAfterEquals(rawString);

    // Get members of each 'member' (recipe)

    std::vector<std::string> individualMembers = extractIndividualMembers(rawString);  // Essentially a list of recipes
    // std::cout << "SIZE: " << individualMembers.size() << std::endl; // (Test statement)


    // stringstream to store output of final processing
    std::ostringstream rawStringStream;


    // Processing individual members
    std::cout << '{' << std::endl; // Opening containing bracket
    for (size_t i = 0; i < individualMembers.size(); ++i) {

        // Get normal recipe, trim expensive one, trim crafting_machine_tint
        auto newMember = getNormal(individualMembers[i]);
        auto newerMember = removeCraftingMachineTint(newMember);

        // Replace member with updated one with normal recipe and extra removed stuff
        individualMembers[i] = newerMember;

        // This gets put in roughCut.txt eventually
        std::string outputString = individualMembers[i]; // Compare this to newer versions below for correctness


        // This is the ROUGHCUT output for now, everything else is essentially runtime debugging
        if (!checkForLastCommaBool) {
            rawStringStream << outputString << ',' << std::endl;
        } 
        else {
            rawStringStream << outputString << std::endl;
        }

        // WE ARE USING THE ROUGHCUT AS THE STARTING POINT FROM HERE
        // No more cleaning up input, only output
        // Now we are assembling our own members from the initial data
        // We may compare afterwards to ensure correctness

        std::string energyOutput = getEnergy(individualMembers[i]);
        std::string ingredientsOutput = getIngredients(individualMembers[i]);
        std::string nameOutput = getRecipeName(individualMembers[i]);
        std::string resultsOutput = getResults(individualMembers[i]);


        std::cout << removeQuotes(nameOutput) << std::endl << removeQuotes(ingredientsOutput) << std::endl << energyOutput << std::endl << removeQuotes(resultsOutput) << std::endl << std::endl;

        // LINE BY LINE TEST STATEMENTS

        //// Print each member
        // std::cout << ingredientsTest << "," << std::endl;


        // std::cout << "THE PREVIOUS MEMBER IS NAMED X HAS " << ingredientsCategory <<" INGREDIENTS AND PRODUCES Z WITH E ENERGY REQUIRED" << std::endl; // Main test statement

        // END OF LOOP TEST STATEMENTS
        //if (i == (individualMembers.size() - 1))
        //{
        //    std::cout << individualMembers.size() << std::endl;
        //}

    }
    std::cout << '}' << std::endl; // Closing containing bracket

    std::string outString = rawStringStream.str();


    // Write to a .txt file
    std::ostringstream outStringStream;
    for (const auto& str : outString) {
        outStringStream << str;
    }

    // Get the final raw string
    std::string roughCutString = outStringStream.str();


    writeToTxtFile(roughCutString, "roughCut.txt");

    return 0;
}
