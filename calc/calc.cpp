#include <string>
#include <iostream>
#include <cctype>
#include <vector>
#include <cmath>

// Tmp for Function::testTrieRemoval();
#include "tokens/Function.h"
#include "tokens/Token.h"

// #define DONT_PRINT_PARSED

#include "Parser.h"
#include "Printer.h"
#include "Sanitizer.h"
#include "Evaluator.h"


int enterMultipleEquationsMode();
int calculate(std::string& equation);
std::string getInput(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    Function::testTrieRemoval();

    std::cout.precision(12);

    // [Get input]

    std::string equation{getInput(argc, argv)};
// Might want to make this a function inside Parser.h
#ifndef DONT_PRINT_PARSED
    std::cout << "[Input]: " << equation << "\n";
#endif

    if (equation.empty())
        return enterMultipleEquationsMode();

    return calculate(equation);
}

int enterMultipleEquationsMode()
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
        // else if (calculate(equation))
        //     return -1;
        else
            calculate(equation);
    }
    return 0;
}

int calculate(std::string& equation)
{
    // Ensure there is an input/equation
    if (equation.empty())
    {
        // Instead of giving the user an error when there are no arguments, I might want to go into a mode where multiple equations can be input separately until the user is done.  With this, I could also add syntax for "[A-Za-z] = ..." (storing results into variables), along with a special keyword for the previous output, like "prev" (there are probably better ones).
        std::cout << "Missing equation - proper format: 'calc \"{equation}\"'\n";
        return 0;
    }


    // [Initial parse pass-through]

    std::vector<Token*> tokens;
    Parser::initialParse(equation, tokens);

    // Printing to ensure that parsing is being done correctly
    Printer::printParsed(tokens, "Parsed");


    // [Malformed Input Checks]

    // HAD to move part of these checks to occur after splitting functions, otherwise replacing functions with constants where relevant would happen AFTER the checks which would lead to problems like e*1 being invalid (it thinks that e should be a function at that point)
    if (Sanitizer::checkForEarlyMalformedInput(tokens))
        return -1;


    // [Equation Sanitization] (might want to use a different word, like Cleaner or Reinterpreter; also, might want to do these in a different order)

    // if (sanitizeEquation(tokens))
    if (Sanitizer::sanitizeEquation(tokens))
        return -1;


    // long double result{evaluate(tokens, 0, tokens.size())};
    long double result{Evaluator::evaluate(tokens, 0, tokens.size())};
    // Not entirely sure how I want to handle outputing this yet, as I want stuff like sin(pi) to output 0 instead of something like "1.22465e-16," but I also want to be very accurate; will likely require some fine tuning
    // std::cout << "[Result]: " << result << '\n';
    // Might want to make the output format small/large numbers slightly differently than how it currently formats like "3.48237e-2"; maybe use "E" or "*10^_" instead
    std::cout << "[Result]: " << (std::abs(result) < 1E-12L ? 0.0L : result) << '\n';

    return 0;
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

