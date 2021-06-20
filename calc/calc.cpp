#include <string>
#include <iostream>
#include <cctype>
#include <vector>

#include "tokens/Token.h"

// #define DONT_PRINT_PARSED

#include "Parser.h"
#include "Printer.h"
#include "Sanitizer.h"
#include "Evaluator.h"


std::string getInput(int argc, char* argv[]);
void enterMultipleEquationsMode();
bool calculate(std::string& equation);

int main(int argc, char* argv[])
{
    std::cout.precision(12);

    // [Get input]

    std::string equation{getInput(argc, argv)};
// Might want to make this a function inside Parser.h
#ifndef DONT_PRINT_PARSED
    std::cout << "[Input]: " << equation << "\n";
#endif

    if (equation.empty())
    {
        enterMultipleEquationsMode();
        return 0;
    }

    return calculate(equation) - 1;
}

std::string getInput(int argc, char* argv[])
{
    std::string equation;
    // Combines all parameters into one
    for (int i = 1; i < argc; i++)
        equation.append(argv[i]);
    for (char& c : equation)
        c = std::tolower(c);
    return equation;
}

void enterMultipleEquationsMode()
{
    std::cout << "Entering Multiple Equations Mode.  Type exit/quit to leave this mode.\n";
    bool isDone{};
    while (!isDone)
    {
        std::cout << "> ";
        std::string equation;
        std::getline(std::cin, equation);
        for (char& c : equation)
            c = std::tolower(c);
        if (equation == "exit" || equation == "quit")
            isDone = true;
        else
            calculate(equation);
    }
}

bool calculate(std::string& equation)
{
    // Ensure there is an input/equation
    if (equation.empty())
    {
        // Instead of giving the user an error when there are no arguments, I might want to go into a mode where multiple equations can be input separately until the user is done.  With this, I could also add syntax for "[A-Za-z] = ..." (storing results into variables), along with a special keyword for the previous output, like "prev" (there are probably better ones).
        // TODO: This cout doesn't make sense while in multiple equations mode
        std::cout << "Missing equation - proper format: 'calc \"{equation}\"'\n";
        return false;
    }


    // [Initial parse pass-through]

    std::vector<Token*> tokens;
    Parser::initialParse(equation, tokens);

    // Printing to ensure that parsing is being done correctly
    Printer::printParsed(tokens, "Parsed");

    // Handle deletion of a custom constant (moving this to something like Number::handleCustomConstantDeletion() would be inconvenient and rather pointless)
    if(tokens.size() == 2 && tokens[0]->tokenType == Token::TokenType::Function && ((Function*)tokens[0])->name == "delete" && tokens[1]->tokenType == Token::TokenType::Function)
        return Number::tryRemoveCustomConstant(((Function*)tokens[1])->name);

    // Might want to try and check to make sure that the custom constant is a valid name here before actually evaluating the right hand side of the = sign
    // Instead of hardcoding the equals sign, I could instead allow assigning of variables within the equation like a = 5 * (3 = b), though this would require me to not convert constants to constants early on and instead create a Token type of Constant
    std::string customConstantName{Number::handleCustomConstantCreation(tokens)};

    // Printing to ensure that Custom constant creation is being handled correctly
    Printer::printParsed(tokens, "Custom Constant Creation Handled ('[variable name] = ' Removed from Start of Equation if Relevant)");

    if (!Operator::ensureProperDoubleOperators(tokens))
        return false;

    // Operator::replaceEquals(tokens);
    Operator::replaceDoubleOperators(tokens);

    // Printing to ensure that replacing of equals signs was done correctly
    Printer::printParsed(tokens, "Replaced '==' with '='");


    // [Malformed Input Checks]

    // HAD to move part of these checks to occur after splitting functions, otherwise replacing functions with constants where relevant would happen AFTER the checks which would lead to problems like e*1 being invalid (it thinks that e should be a function at that point)
    if (!Sanitizer::checkForEarlyMalformedInput(tokens))
        return false;


    // [Equation Sanitization] (might want to use a different word, like Cleaner or Reinterpreter; also, might want to do these in a different order)

    if (!Sanitizer::sanitizeEquation(tokens))
        return false;


    // [Equation Evaluation]

    long double result{Evaluator::evaluate(tokens, 0, tokens.size())};

    // Might want to make the output format small/large numbers slightly differently than how it currently formats like "3.48237e-2"; maybe use "E" or "*10^_" instead
    // std::cout << "[Result]: " << (std::abs(result) < 1E-12L ? 0.0L : result) << '\n';

    // Not entirely sure how I want to handle outputing this yet, as I want stuff like sin(pi) to output 0 instead of something like "1.22465e-16," but I also want to be very accurate; will likely require some fine tuning
    // Instead (for now), I should handle sufficiently small numbers on a case by case basis based solely on a function being a trig function
    std::cout << "[Result]: " << result << '\n';


    // [Custom Constant Creation Assigning]

    if (!customConstantName.empty())
        Number::tryAddCustomConstant(customConstantName, result);


    // [Cleanup]

    for (int i = 0; i < tokens.size(); i++)
        delete tokens[i];

    return true;
}
