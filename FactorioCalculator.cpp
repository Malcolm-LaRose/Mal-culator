#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <vector>
#include <string>

// Forward declaration of Recipe class to allow Item class to reference it
class Recipe;

// Item represents a component in the production chain
struct Item {
    std::string name;           // Name of the item
    double amount;              // Amount required or produced
    const Recipe* recipe;       // Pointer to the recipe for this item

    // Constructor for Item
    Item(const std::string& itemName, double itemAmount, const Recipe* itemRecipe)
        : name(itemName), amount(itemAmount), recipe(itemRecipe) {}
};

// Recipe represents a crafting recipe in the production chain
class Recipe {
public:
    std::string outputItem;                                     // Name of the item produced
    double craftingTime;                                        // Time required to craft the item
    std::vector<std::pair<double, const Recipe*>> inputItems;    // Input items along with their amounts and corresponding recipes
    int outputItems;                                            // Number of items produced per crafting cycle

    // Constructor for Recipe with dependencies
    Recipe(const std::string& output, double time, const std::vector<std::pair<double, const Recipe*>>& inputRecipes, int outputCount)
        : outputItem(output), craftingTime(time), inputItems(inputRecipes), outputItems(outputCount) {}

    // Default constructor
    Recipe() : craftingTime(0.0), outputItems(0) {}
};

// ProductionChain manages the recipes and calculates production requirements
class ProductionChain {
public:
    // Adds a recipe to the production chain
    void addRecipe(const Recipe& recipe) {
        recipes[recipe.outputItem] = recipe;
    }

    // Retrieves the recipes map
    const std::unordered_map<std::string, Recipe>& getRecipes() const {
        return recipes;
    }

    // Calculates production requirements for a given output item and production rate
    void calculateRequirements(const std::string& outputItem, double prodRate) const {
        calculateRequirementsRecursive(outputItem, prodRate);
    }

private:
    std::unordered_map<std::string, Recipe> recipes;    // Map to store recipes by output item name

    // Recursive function to calculate production requirements
    void calculateRequirementsRecursive(const std::string& item, double pRate) const {
        auto it = recipes.find(item);
        if (it == recipes.end()) {
            std::cout << std::endl;
            return;
        }

        const Recipe& recipe = it->second;


        // Iterate through input items in the recipe
        for (const auto& inputItem : recipe.inputItems) {
            double requiredAmount = inputItem.first * pRate / recipe.outputItems;

            std::cout << "To produce " << pRate << " " << item << " per second, you need:" << std::endl;
            std::cout << requiredAmount << " " << inputItem.second->outputItem << " per second" << std::endl << std::endl;

            // Recursive call for each input item
            calculateRequirementsRecursive(inputItem.second->outputItem, requiredAmount);
        }
    }
};

void help(const ProductionChain& productionChain)
{
    std::cout << "Here are the currently supported items:" << std::endl;

    for (const auto& pair : productionChain.getRecipes()) {
        std::cout << pair.first << std::endl;
    }

    std::exit(0);
}

std::string getInputItem(const ProductionChain& productionChain)
{
    std::cout << "Please enter an item (produces x per sec at crafting speed 1): " << std::endl;

    std::string desiredItem = "help";
    std::getline(std::cin, desiredItem);


    if ((desiredItem == "help") || (desiredItem == "HELP") || (desiredItem == "h") || (desiredItem == ""))
    {   
        help(productionChain);
        return "Thanks for asking here!";
    }

    return desiredItem;

}

int getInputRate()
{
    std::cout << "Please enter a production rate (x per second): " << std::endl;

    int desiredRate = 1;
    std::cin >> desiredRate;

    return desiredRate;

}


int main() {
    ProductionChain productionChain;

    // Define basic recipes
    // Recipe codename("Real Name", time to craft in s, {refs to deeper recipes}, number of output for given input)
    Recipe ironPlateRecipe("Iron Plate", 3.2, {}, 1);
    Recipe copperPlateRecipe("Copper Plate", 3.2, {}, 1);

    // Define reCURSED recipes --> Roughly grouped
    // Recipe codename("Real Name", time to craft in s, {numInput, refs to deeper recipes}, number of output for given input)
    Recipe ironGearRecipe("Iron Gear", 0.5, { {2.0, &ironPlateRecipe} }, 1);
    Recipe steelPlateRecipe("Steel Plate", 16.0, { {5.0, &ironPlateRecipe} }, 1);

    Recipe copperWireRecipe("Copper Wire", 0.5, { {1.0, &copperPlateRecipe} }, 2);
    Recipe electronicCircuitRecipe("Electronic Circuit", 0.5, { {1.0, &ironPlateRecipe}, {3.0, &copperWireRecipe} }, 1);

    // Add recipes to the production chain
    productionChain.addRecipe(ironPlateRecipe);
    productionChain.addRecipe(copperPlateRecipe);
    productionChain.addRecipe(steelPlateRecipe);
    productionChain.addRecipe(ironGearRecipe);
    productionChain.addRecipe(copperWireRecipe);
    productionChain.addRecipe(electronicCircuitRecipe);

    // Calculate requirements for a given output item and amount per unit time (default s)
    productionChain.calculateRequirements("Electronic Circuit", 1.0);
    productionChain.calculateRequirements("Steel Plate", 10);

    std::string desItem = getInputItem(productionChain);
    int desRate = getInputRate();
    productionChain.calculateRequirements(desItem, desRate);

    return 0;
}