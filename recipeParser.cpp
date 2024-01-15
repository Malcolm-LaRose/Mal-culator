/*
/c lines = {} for k, v in pairs(game.recipe_prototypes) do   table.insert(lines, serpent.line({v.name,  v.ingredients, v.products})) end game.write_file("factorio-recipes.txt", table.concat(lines, "\n"))
*/


// Gonna want to pare down the information with reg expr first
// Then use reg expr to get the information we need and store it in variables



// Read in data { {}, {}, ... , {}, {} }
// Store each {} as its own raw string
// parse through the raw string to get desired data, account for variable parsing needs and whatnot
//		normal v expensive
//		desired data 


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <cctype>  // Include the header for std::isspace



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

void removeCraftingMachineTint(std::string& text) {
    std::regex pattern(R"(crafting_machine_tint\s*=\s*\{[^{}]*\})");

    // Use regex_replace to replace matches with an empty string
    text = std::regex_replace(text, pattern, "");
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

    /*std::string testtext = R"(

  {
    type = "recipe",
    name = "steam-engine",
    normal =
    {
      ingredients =
      {
        {"iron-gear-wheel", 8},
        {"pipe", 5},
        {"iron-plate", 10}
      },
      result = "steam-engine"
    },
    expensive =
    {
      ingredients =
      {
        {"iron-gear-wheel", 10},
        {"pipe", 5},
        {"iron-plate", 50}
      },
      result = "steam-engine"
    }
  },
  {
    type = "recipe",
    name = "iron-gear-wheel",
    normal =
    {
      ingredients = {{"iron-plate", 2}},
      result = "iron-gear-wheel"
    },
    expensive =
    {
      ingredients = {{"iron-plate", 4}},
      result = "iron-gear-wheel"
    }
  }


)";*/

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


// ONE LINE CATEGORIES ONLY!!!
std::vector<std::string> catsToRemove = { "type", "category", "enabled", "order", "allow_decomposition", "main_product", "subgroup", "requester_paste_multiplier"};

removeLeadingWhitespace(rawString);
ensureNewlineAfterBrace(rawString);
removeLeadingWhitespace(rawString);

for (auto& vi : catsToRemove) {
    removeLineStartingWith(rawString, vi);
}

removeCraftingMachineTint(rawString);

    std::vector<std::string> individualMembers = extractIndividualMembers(rawString);
    std::cout << "SIZE: " << individualMembers.size() << std::endl;


    std::cout << '{' << std::endl; // Opening containing bracket
    for (size_t i = 0; i < individualMembers.size(); ++i) {
        // std::cout << "Original member:\n" << individualMembers[i] << '\n';  // Debug line

        auto newMember = getNormal(individualMembers[i]);

        // std::cout << "Modified member:\n" << newMember << '\n';  // Debug line

        individualMembers[i] = newMember;

        std::cout << individualMembers[i];

        // Check if it's not the last member before printing the comma
        if (i < individualMembers.size() - 1) {
            std::cout << ',';
        }

        std::cout << std::endl;
    }
    std::cout << '}' << std::endl; // Closing containing bracket

    std::ostringstream rawStringStream;
    for (const auto& str : rawString) {
        rawStringStream << str;
    }

    // Get the final raw string
    std::string roughString = rawStringStream.str();

    writeToTxtFile(roughString, "roughCut.txt");


    // Display the extracted members test statement
    //for (const auto& member : individualMembers) {
    //    std::cout << "Extracted member: " << member << '\n';
    //}

    return 0;
}
