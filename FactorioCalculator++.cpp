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

    // Default constructor
    Item() : name(""), amount(0.0), recipe(nullptr) {}

    // Constructor for Item
    Item(const std::string& itemName, double itemAmount, const Recipe* itemRecipe = nullptr)
        : name(itemName), amount(itemAmount), recipe(itemRecipe) {
    }
};

// Recipe represents a crafting recipe in the production chain
class Recipe {
public:
    std::string craftedItem;                                     // Name of the item produced
    double craftingTime;                                        // Time required to craft the item in seconds
    std::vector<std::pair<double, const Recipe*>> ingredients;    // Input items along with their amounts and corresponding recipes
    int numItemsCrafted;                                            // Number of items produced per crafting cycle


    // Constructor for Recipe with dependencies
    Recipe(const std::string& output, double time, const std::vector<std::pair<double, const Recipe*>>& inputRecipes, int outputCount)
        : craftedItem(output), craftingTime(time), ingredients(inputRecipes), numItemsCrafted(outputCount) {}


    Recipe(const std::string& output, double time, int outputCount)
        : craftedItem(output), craftingTime(time), numItemsCrafted(outputCount) {}

    // A basic Recipe like iron plate should take an Extraction (e.g. iron ore)


    // Default constructor
    Recipe() : craftingTime(0.0), numItemsCrafted(0) {}

};

class Extraction {
public:
    // Like a Recipe, but suitable for extractionary machines instead of assemblers

    std::string extractedItem;
    double extractionTime;
    int numItemsExtracted;

    Extraction(const std::string& outputName, double time, int outputCount)
        : extractedItem(outputName), extractionTime(time), numItemsExtracted(outputCount) {}


    // Default constructor
    Extraction() : extractionTime(0.0), numItemsExtracted(0) {}

};

// ProductionChain manages the recipes and calculates production requirements
class ProductionChain {
public:
    // Adds a recipe to the production chain (recipe named after the item it crafts)
    void addRecipe(const Recipe& recipe) {
        recipes[recipe.craftedItem] = recipe;
    }

    // Retrieves the recipes map
    const std::unordered_map<std::string, Recipe>& getRecipes() const {
        return recipes;
    }

    // Gets a recipe from recipes by name DOESNT WORK maybe
    Recipe* getRecipe(std::string outpItem) {
        return  (&(recipes.find(outpItem))->second);

    }

    // Gets an item from the production chain by name
    Item& getItem(const std::string& itemName) {
        return items[itemName];
    }

    void calculateRequirements(const std::string& outputItem, double prodRate, double craftingSpeed, int recursionDepth = 0) {
        auto outItem = recipes.find(outputItem); // Gets the recipe for the desired output item for the calculator by name
        const Recipe* outItemRecipe = &outItem->second; // Retrieves (points to) the recipe from the name,recipe pair
        

        // First, say basic requirements
        // If there are ingredients in the recipe (its not a raw item)
        if (!outItemRecipe->ingredients.empty()) {

            std::cout << "To produce " << prodRate << " " << outItemRecipe->craftedItem << " per second crafting at "
                << craftingSpeed << " items per second, you need:" << std::endl;

            for (const auto& inputItem : outItemRecipe->ingredients) { // Loop over all ingredients in the recipe
                double requiredAmount = inputItem.first * (prodRate) / outItemRecipe->numItemsCrafted;

                std::cout << requiredAmount / craftingSpeed << " " << inputItem.second->craftedItem << " per second" << std::endl;
                std::cout << std::endl;
            }

        }

        // Then recursively go through sub ingredients
        // std::cout << "Entering recursive function" << std::endl;
        if (!outItemRecipe->ingredients.empty()) {

            for (const auto& inputItem : outItemRecipe->ingredients) { // Loop over all ingredients in the recipe
                double requiredAmount = inputItem.first * (prodRate) / outItemRecipe->numItemsCrafted;

                // This is using recursion, which works but feels clunky
                // Need to format output
                if (!inputItem.second->ingredients.empty()) {
                    // std::cout << "Going back to calculate requirements for subingredients" << std::endl;
                    calculateRequirements(inputItem.second->craftedItem, requiredAmount, craftingSpeed, recursionDepth + 1);
                    std::cout << std::endl;
                }

                else {
                    // Calculate Total
                    totalRawIngredients[inputItem.second->craftedItem] += requiredAmount / craftingSpeed;
                }

            }
            // std::cout << "Exiting recursive function" << std::endl;
            if (recursionDepth == 0) {
                std::cout << "In total you need: " << std::endl;
                for (const auto& rawIngredients : totalRawIngredients) {
                    std::cout << rawIngredients.second << " " << rawIngredients.first << " per second" << std::endl;
                }
            }
        }

    }

    //// Overloaded function to calculate production requirements for a given output item and production rate using Item
    //// Allows the programmer to use a simpler Item name to manipulate Items
    //void calculateRequirements(const Item& outputItem, double prodRate, double craftingSpeed) const {
    //    calculateRequirementsRecursive(outputItem.recipe, prodRate, craftingSpeed);
    //}



private:
    std::unordered_map<std::string, Recipe> recipes;    // Map to store recipes by output item name
    std::unordered_map<std::string, Item> items;        // Map to store items by name
    mutable std::unordered_map<std::string, double> totalRawIngredients;



};

class craftingMachine {
public:

    std::string type;
    double craftingRate;



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
    std::cout << "Please enter a production rate (x items per second): " << std::endl;

    int desiredRate = 1;
    std::cin >> desiredRate;

    return desiredRate;
}

double getCraftingSpeed()
{
    std::cout << "Please enter a crafting speed (y items per second): " << std::endl;

    double desiredRate = 1.0; // Default 1/sec
    std::cin >> desiredRate;

    return desiredRate;
}

int main() {
    ProductionChain productionChain;

    // Eventually define base items here, like ores and liquids
    Recipe ironOreRecipe("Iron Ore", 0.0, {}, 1);
    Recipe copperOreRecipe("Copper Ore", 0.0, {}, 1);
    Recipe crudeOil("Crude Oil", 0.0, {}, 1);
    Recipe stone("Stone", 0.0, {}, 1);


    // Define basic recipes
    // Recipe codename("Real Name", time to craft in s, {refs to deeper recipes}, number of output for given input)
    Recipe ironPlateRecipe("Iron Plate", 3.2, {/* {1.0, &ironOreRecipe} */}, 1);
    Recipe copperPlateRecipe("Copper Plate", 3.2, {/* {1.0, &copperOreRecipe} */}, 1);

    // Define recursive recipes --> Roughly grouped by depth
    // Recipe codename("Real Name", time to craft in s, {numInput, refs to deeper recipes}, number of output for given input)
    Recipe ironGearRecipe("Iron Gear", 0.5, { {2.0, &ironPlateRecipe} }, 1);
    Recipe ironStickRecipe("Iron Stick", 0.5, { {1.0, &ironPlateRecipe} }, 1);
    Recipe steelPlateRecipe("Steel Plate", 16.0, { {5.0, &ironPlateRecipe} }, 1);

    Recipe pipeRecipe("Pipe", 0.5, { {1.0, &ironPlateRecipe} }, 1);
    Recipe copperWireRecipe("Copper Wire", 0.5, { {1.0, &copperPlateRecipe} }, 2);

    Recipe engineUnitRecipe("Engine Unit", 11.5, { {1.0, &steelPlateRecipe}, {1.0, &ironGearRecipe}, {2.0, &pipeRecipe} }, 1);
    Recipe electronicCircuitRecipe("Electronic Circuit", 0.5, { {1.0, &ironPlateRecipe}, {3.0, &copperWireRecipe} }, 1);

    Recipe flamethrowerTurret("Flamethrower", 20, { {5.0, &engineUnitRecipe}, {15.0, &ironGearRecipe}, {10.0, &pipeRecipe}, {30.0, &steelPlateRecipe} }, 1);

    // Add recipes to the production chain
    productionChain.addRecipe(ironOreRecipe);
    productionChain.addRecipe(copperOreRecipe);
    productionChain.addRecipe(ironPlateRecipe);
    productionChain.addRecipe(copperPlateRecipe);
    productionChain.addRecipe(steelPlateRecipe);
    productionChain.addRecipe(ironGearRecipe);
    productionChain.addRecipe(ironStickRecipe);
    productionChain.addRecipe(copperWireRecipe);
    productionChain.addRecipe(electronicCircuitRecipe);
    productionChain.addRecipe(pipeRecipe);
    productionChain.addRecipe(engineUnitRecipe);
    productionChain.addRecipe(flamethrowerTurret);

    // Retrieve items from the production chain
    // Item& ironPlate = productionChain.getItem("Iron Plate");
    // Item& copperPlate = productionChain.getItem("Copper Plate");
    // Item& ironGear = productionChain.getItem("Iron Gear");
    // Item& steelPlate = productionChain.getItem("Steel Plate");
    // Item& copperWire = productionChain.getItem("Copper Wire");
    // Item& electronicCircuit = productionChain.getItem("Electronic Circuit");
    // Item& ironStick = productionChain.getItem("Iron Stick");
    // Item& pipe = productionChain.getItem("Pipe");
    // Item& engineUnit = productionChain.getItem("Engine Unit");

    // Calculate requirements for a given output item and amount per unit time (default s)
    // productionChain.calculateRequirements(ironPlate, 5);

    std::string desItem = getInputItem(productionChain);
    int desRate = getInputRate();
    double craftSpeed = getCraftingSpeed();

    system("cls");

    productionChain.calculateRequirements(desItem, desRate, craftSpeed);

    system("pause");

    return 0;
}
