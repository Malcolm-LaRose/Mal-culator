/*

    Written by Malcolm LaRose for C++/17 in January, the year of our Lord, 2024, the 16th day.
    Free to use and edit, but please attribute me if you publish anything based on this

TODO:
    Move to utilities file to be used by factoriocalc
    Finish cleaning up recipe list
        // Clean up braces and newlines
    Figure out how to extract data
    Figure out how to put data into Recipes

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



std::string formatToSingleLine(const std::string& input) {
    std::istringstream iss(input);
    std::ostringstream oss;

    std::string line;
    while (std::getline(iss, line)) {
        // Append the line to the output string, removing leading and trailing whitespaces
        oss << line;
    }

    return oss.str();
}


// FIX THIS TO OUTPUT A STRING PROBABLY
void formatIngredientsOnOneLine(std::string& text) {
    // Find the position of "ingredients"
    size_t ingredientsPos = text.find("ingredients");

    std::string ingContents = extractNestedBraces(text.substr(ingredientsPos));
    // std::cout << ingContents;
    if (ingredientsPos != std::string::npos) {
        // Find the opening '{' after "ingredients"
        size_t openBracePos = text.find('{', ingredientsPos);

        // Find the closing '}' after "ingredients"
        size_t closeBracePos = text.find('}', ingredientsPos);

        if (openBracePos != std::string::npos && closeBracePos != std::string::npos) {
            // Extract the contents of "ingredients" including the opening and closing braces
            std::string ingredientsContents = extractNestedBraces(text.substr(ingredientsPos));

            // Remove newline characters and whitespace within the ingredients contents
            std::string outGredients = formatToSingleLine(extractNestedBraces(text.substr(ingredientsPos)));

            // Replace the original ingredients category with the formatted one
            text.replace(ingredientsPos,ingredientsContents.length(), outGredients);
        }
    }
}



std::pair<std::string, std::string> getIngredients(std::string& inputMember) {
    // Position of ingredients string
    size_t ingPos = inputMember.find("ingredients");
    // std::cout << "ingPos: " << ingPos << std::endl;

    if (ingPos != std::string::npos) {
        // Extract the contents of "ingredients" including the braces
        std::string ingContents = extractNestedBraces(inputMember.substr(ingPos));
        // Remove ingredients category and the extra set of braces from the original member
        std::string modifiedMember = inputMember.substr(0, ingPos) + inputMember.substr(ingContents.length(),inputMember.length());



        std::pair<std::string, std::string> outputPair(ingContents, modifiedMember);
        return outputPair;
    }

    // If "ingredients" is not found, panic
    return { {"panic"}, {"mode"} };
}


std::string getName(std::string& inputMember) {

    // Take a member
    // Find name string
    // Find the position of "name"
    size_t CMTPos = inputMember.find("name");
    // Extract important info
    // Return important info

    return "";
}

std::string getResults(std::string& inputMember) {

    // Check for results and result count
    // if no result count, result count = 1

    return "";
}

std::string getEnergy(std::string& inputMember) {

    // Check for energy required
    // If no energy required, energy required = 1

    return "";
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
    // Might be easiest to trim then grab

//    std::cout << "Raw string content:\n" << rawString << std::endl;


//std::string testtextTwo = R"(
//
//  {
//    type = "recipe",
//    name = "steam-engine",
//    normal =
//    {
//      ingredients =
//      {
//        {"iron-gear-wheel", 8},
//        {"pipe", 5},
//        {"iron-plate", 10}
//      },
//      result = "steam-engine"
//    },
//    expensive =
//    {
//      ingredients =
//      {
//        {"iron-gear-wheel", 10},
//        {"pipe", 5},
//        {"iron-plate", 50}
//      },
//      result = "steam-engine"
//    }
//  },
//  {
//    type = "recipe",
//    name = "iron-gear-wheel",
//    normal =
//    {
//      ingredients = {{"iron-plate", 2}},
//      result = "iron-gear-wheel"
//    },
//    expensive =
//    {
//      ingredients = {{"iron-plate", 4}},
//      result = "iron-gear-wheel"
//    }
//  },
//  {
//    type = "recipe",
//    name = "electronic-circuit",
//    normal =
//    {
//      ingredients =
//      {
//        {"iron-plate", 1},
//        {"copper-cable", 3}
//      },
//      result = "electronic-circuit"
//    },
//    expensive =
//    {
//      ingredients =
//      {
//        {"iron-plate", 2},
//        {"copper-cable", 8}
//      },
//      result = "electronic-circuit"
//    }
//  }
//
//)";

//std::string newTestCaseString = R"( 
//
//{
//name = "speed-module",
//ingredients =
//{
//{"advanced-circuit", 5},
//{"electronic-circuit", 5}
//},
//energy_required = 15,
//result = "speed-module"
//},
//{
//name = "advanced-oil-processing",
//energy_required = 5,
//ingredients =
//{
//{type="fluid", name="water", amount=50},
//{type="fluid", name="crude-oil", amount=100}
//},
//results=
//{
//{type="fluid", name="heavy-oil", amount=25},
//{type="fluid", name="light-oil", amount=45},
//{type="fluid", name="petroleum-gas", amount=55}
//},
//icon = "__base__/graphics/icons/fluid/advanced-oil-processing.png",
//icon_size = 64, icon_mipmaps = 4,
//},
//{
//name = "refined-hazard-concrete",
//energy_required = 0.25,
//ingredients =
//{
//{"refined-concrete", 10}
//},
//result= "refined-hazard-concrete",
//result_count = 10
//},
//{
//name = "electric-engine-unit",
//energy_required = 10,
//ingredients =
//{
//{"engine-unit", 1},
//{type="fluid", name= "lubricant", amount = 15},
//{"electronic-circuit", 2}
//},
//result = "electric-engine-unit",
//},
//
//)";

    // Functions that act on the whole raw string
    // 
    // ONE LINE CATEGORIES ONLY!!!
    std::vector<std::string> catsToRemove = { "type", "category", "enabled", "order", "allow_decomposition", "main_product", "subgroup", "requester_paste_multiplier"};

    removeLeadingWhitespace(rawString);
    ensureNewlineAfterBrace(rawString);
    removeLeadingWhitespace(rawString);

    for (auto& vi : catsToRemove) {
        removeLineStartingWith(rawString, vi);
    }
    removeLineStartingWith(rawString, "icon");

    removeNewlineAfterEquals(rawString);
    formatIngredientsOnOneLine(rawString);

    // Get members of each 'member' (recipe)

    std::vector<std::string> individualMembers = extractIndividualMembers(rawString);
    std::cout << "SIZE: " << individualMembers.size() << std::endl; // (Test statement)


    // stringstream to store output of final processing
    std::ostringstream rawStringStream;
    std::ostringstream smoothStringStream;


    // Processing individual members
    std::cout << '{' << std::endl; // Opening containing bracket
    for (size_t i = 0; i < individualMembers.size(); ++i) {
        // std::cout << "Original member:\n" << individualMembers[i] << '\n';  // Debug line
        
        std::cout << "NEW MEMBER: " << std::endl << std::endl;

        // Get normal recipe, trim expensive one, trim crafting_machine_tint
        auto newMember = getNormal(individualMembers[i]);
        auto newerMember = removeCraftingMachineTint(newMember);
        auto newestMember = "";

        // std::cout << "Modified member:\n" << newMember << '\n';  // Debug line

        // Replace member with updated one with normal recipe and extra removed stuff
        individualMembers[i] = newerMember;


        // This is the ROUGHCUT output for now, everything else is essentially runtime debugging
        if (!checkForLastCommaBool) {
            rawStringStream << individualMembers[i] << ',' << std::endl;
        }
        else {
            rawStringStream << individualMembers[i] << std::endl;
        }

        //std::string ingredientsCategory = getIngredients(individualMembers[i]).first;
        std::string memberWithoutIngredients = getIngredients(individualMembers[i]).second;

       // individualMembers[i] = memberWithoutIngredients;

        // Below here we want to extract the data from each member
            // We should probably do this by identifying each category and placing them into a new member
            // We can delete each category from the member as we collect it
            // So reconstruct the members with the relevant information while we deplete the originals
        // Need to treat ingredients first, as it has the weirdest structure and removing it from

        

        // Print each member
        std::cout << individualMembers[i];

        // Check if it's not the last member before printing the comma
        checkForLastComma(individualMembers, i);

        // Newline after comma
        std::cout << std::endl;

        // std::cout << "THE PREVIOUS MEMBER IS NAMED X HAS " << ingredientsCategory <<" INGREDIENTS AND PRODUCES Z WITH E ENERGY REQUIRED" << std::endl; // Main test statement

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



    // Display the extracted members test statement
    //for (const auto& member : individualMembers) {
    //    std::cout << "Extracted member: " << member << '\n';
    //}

    return 0;
}
