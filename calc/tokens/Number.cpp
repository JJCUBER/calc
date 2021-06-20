#include <numbers>
// #include <algorithm>
#include <iostream>

#include "Number.h"

#include "Function.h"
#include "Operator.h"

const std::unordered_map<std::string, long double> Number::constants
{
    {"e", std::numbers::e},
    {"pi", std::numbers::pi},
    {"phi", std::numbers::phi}
};
std::unordered_map<std::string, long double> Number::customConstants;


std::pair<bool, long double> Number::getConstant(const std::string& s)
{
    auto it{constants.find(s)};
    if (it == constants.end())
    {
        it = customConstants.find(s);
        if (it == customConstants.end())
            // Neither unordered map contains s
            return {false, 0.0L};
    }
    return {true, it->second};
}

bool Number::tryAddCustomConstant(const std::string& s, long double n)
{
    // check if in Function::functionNames or Number::constants, add it only if it isn't; it's okay to override an existing custom constant (might want to issue a warning message though?)
    // should check if already an existing custom constant, if it is, then update its value in Number::customConstants and don't bother adding it to the trie (only try to add it to the trie if it is new)

    auto customConstantsIt = customConstants.find(s);
    if (customConstantsIt != customConstants.end())
    {
        customConstantsIt->second = n;
        return true;
    }

    /* Instead of doing this, I could just traverse the trie and see if it is already inside of it, if it is that means that there is a builtin function or constant with that name, as we already checked to see if it is in the customConstants umap
    auto constantsIt = constants.find(s);
    if (constantsIt != constants.end())
    {
        // cout that there is a builtin constant with this name
        return false;
    }

    auto constantsIt = constants.find(s);
    if (constantsIt != constants.end())
    {
        // cout that there is a builtin constant with this name
        return false;
    }
    */
    if (Function::trie.find(s))
    {
        std::cout << "Error: Invalid Custom Constant Name - there is already a builtin constant/function with the name '" << s << "'\n";
        return false;
    }

    customConstants[s] = n;
    Function::trie.insert(s);
    return true;
}

bool Number::tryRemoveCustomConstant(const std::string& s)
{
    // first check to make sure that it is in custom constants, return false if it isn't
    auto it{customConstants.find(s)};
    if (it == customConstants.end())
    {
        std::cout << "Error: Invalid Custom Constant Name - there is no custom constant with the name '" << s << "'\n";
        return false;
    }
    customConstants.erase(it);
    Function::trie.remove(s);

    return true;
}

std::string Number::handleCustomConstantCreation(std::vector<Token*>& tokens)
{
    // if(tokens.size() > 2 && tokens[0]->tokenType == TokenType::Function && tokens[1]->tokenType == TokenType::Operator && ((Operator*)tokens[1])->symbol == '=')
    // if(tokens.size() <= 2 || tokens[0]->tokenType != TokenType::Function || tokens[1]->tokenType != TokenType::Operator || ((Operator*)tokens[1])->symbol != '=')
    // Must have at least 3 tokens, second token must be equals sign, and third token must not be equals sign (otherwise we would be doing something like 'a == ...' which is not the same as assigning a value to 'a')
    if(tokens.size() <= 2 ||
       tokens[0]->tokenType != TokenType::Function ||
       tokens[1]->tokenType != TokenType::Operator || ((Operator*)tokens[1])->orderOfOp != Operator::OrderOfOpType::Equal ||
       tokens[2]->tokenType == TokenType::Operator && ((Operator*)tokens[2])->orderOfOp == Operator::OrderOfOpType::Equal)
        return "";
    std::string customConstantName{((Function*)tokens[0])->name};
    delete tokens[0];
    delete tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    return customConstantName;
}
