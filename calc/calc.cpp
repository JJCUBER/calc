#include <string>
#include <iostream>
#include <vector>
#include <cctype>

#include "tokens/Token.h"
#include "tokens/Number.h"
#include "tokens/Operator.h"
#include "tokens/Grouping.h"
#include "tokens/Function.h"

// #define DONT_PRINT_PARSED

int enterMultipleEquationsMode();
int calculate(std::string& equation);
std::string getInput(int argc, char* argv[]);
void initialParse(std::string& equation, std::vector<Token*>& tokens);
void printParsed(const std::vector<Token*>& tokens, const std::string& reason = "");
int checkForMalformedInput(const std::vector<Token*>& tokens);
int sanitizeEquation(std::vector<Token*>& tokens);
int evaluate(const std::vector<Token*>& tokens);

int main(int argc, char* argv[])
{
    // [Get input]

    std::string equation{getInput(argc, argv)};
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
            c = tolower(c);
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
    initialParse(equation, tokens);

    // Printing to ensure that parsing is being done correctly
    printParsed(tokens, "Parsed");


    // [Malformed Input Checks]

    if (checkForMalformedInput(tokens))
        return -1;


    // [Equation Sanitization] (might want to use a different word, like Cleaner or Reinterpreter; also, might want to do these in a different order)

    if (sanitizeEquation(tokens))
        return -1;


    return 0;
}

std::string getInput(int argc, char* argv[])
{
    std::string equation;
    // Combines all parameters into one
    for (int i = 1; i < argc; i++)
        equation.append(argv[i]);
    for (char& c : equation)
        c = tolower(c);
    return equation;
}

void initialParse(std::string& equation, std::vector<Token*>& tokens)
{
    std::string token{};
    bool isNumber{};
    for (char c : equation)
    {
        if (c == '.' || isdigit(c))
        {
            if (!isNumber && !token.empty())
            {
                tokens.push_back(new Function{token});
                token.clear();
            }

            token.push_back(c);
            isNumber = true;
            continue;
        }

        if (isNumber)
        {
            tokens.push_back(new Number{std::stold(token)});
            token.clear();
        }

        isNumber = false;
        
        if (!isalpha(c))
        {
            if (!token.empty())
            {
                tokens.push_back(new Function{token});
                token.clear();
            }

            if (c != ' ')
                tokens.push_back(c == '(' || c == ')' ? (Token*)new Grouping{c} : new Operator{c});
            continue;
        }

        token.push_back(c);
    }
    if (!token.empty())
        tokens.push_back(isNumber ? (Token*)new Number{std::stold(token)} : new Function{token});
}

// There is probably a better word to use here than "reason"
void printParsed(const std::vector<Token*>& tokens, const std::string& reason)
{
#ifdef DONT_PRINT_PARSED
    return;
#endif

    std::string parsed;
    for (Token* t : tokens)
    {
        switch (t->tokenType)
        {
        case Token::TokenType::Number:
            parsed += std::to_string(((Number*)t)->n);
            break;
        case Token::TokenType::Operator:
            parsed.push_back(((Operator*)t)->symbol);
            break;
        case Token::TokenType::Grouping:
            parsed.push_back(((Grouping*)t)->symbol);
            break;
        case Token::TokenType::Function:
            parsed += ((Function*)t)->name;
            break;
        }
        parsed.push_back('_');
    }
    std::cout << (reason.empty() ? "" : '[' + reason + "]: ") << parsed << "\n";
}

int checkForMalformedInput(const std::vector<Token*>& tokens)
{
    // Starting with any operator
    if (tokens[0]->tokenType == Token::TokenType::Operator)
    {
        std::cout << "Error: Malformed Input - input starts with the operator (or unknown symbol) '" << ((Operator*)tokens[0])->symbol << "'\n";
        return -1;
    }

    // Ending with any operator other than '!'
    if (tokens.back()->tokenType == Token::TokenType::Operator)
    {
        Operator* op{(Operator*)tokens.back()};
        if (op->symbol != '!')
        {
            std::cout << "Error: Malformed Input - input ends with the operator (or unknown symbol) '" << op->symbol << "'\n";
            return -1;
        }
    }

    // TODO: Sort out my lack of consistency with regard to the order of prev (i - 1) and curr (i) throughout the codebase
    // Might also want to invert ifs involving a continue and return path/branch inside of them to be more consistent
    for (int i = 1; i < tokens.size(); i++)
    {
        // Consecutive numbers
        if (tokens[i]->tokenType == Token::TokenType::Number && tokens[i - 1]->tokenType == Token::TokenType::Number)
        {
            std::cout << "Error: Malformed Input - input has consecutive numbers: '" << ((Number*)tokens[i - 1])->n << "' '" << ((Number*)tokens[i])->n << "'\n";
            return -1;
        }

        // Might want to add handling of consecutive operators here for stuff like '5*-4,' '5++4,' '5--4,' '5---4,' etc. (might want to warn about extra +'s and -'s)
        // Will also need to handle functions after taking into account ^, as in 'floor(+5),' 'floor(-5),' and 'floor(---5)' should all be valid, but not 'floor(*5)'

        // Consecutive operators other than ![_] or [_][+-]
        if (tokens[i - 1]->tokenType == Token::TokenType::Operator && tokens[i]->tokenType == Token::TokenType::Operator)
        {
            Operator *prevOp{(Operator*)tokens[i - 1]}, *currOp{(Operator*)tokens[i]};
            // if (prevOp->orderOfOp == 2 && currOp->orderOfOp != 2)
            if (prevOp->symbol != '!' && currOp->orderOfOp != 2)
            {
                std::cout << "Error: Malformed Input - input has consecutive operators other than '![_]' or '[_][+-]': '" << prevOp->symbol << currOp->symbol << "'\n";
                return -1;
            }
            continue;
        }

        // Function followed by a symbol other than '^'
        if(tokens[i - 1]->tokenType == Token::TokenType::Function && tokens[i]->tokenType == Token::TokenType::Operator)
        {
            Operator* currOp{(Operator*)tokens[i]};
            if(currOp->symbol != '^')
            {
                std::cout << "Error: Malformed Input - input has an operator other than '^' directly after a function: '" << ((Function*)tokens[i - 1])->name << currOp->symbol << "'\n";
                return -1;
            }
            continue;
        }

        // Inside of group starts with any operator
        if (tokens[i - 1]->tokenType == Token::TokenType::Grouping)
        {
            Grouping* grouping{(Grouping*)tokens[i - 1]};
            if (grouping->isOpen && tokens[i]->tokenType == Token::TokenType::Operator)
            {
                std::cout << "Error: Malformed Input - input has an operator directly after a grouping: '" << grouping->symbol << ((Operator*)tokens[i])->symbol << "'\n";
                return -1;
            }
        }
        // Inside of group ends with any operator other than '!'
        else if (tokens[i]->tokenType == Token::TokenType::Grouping && !((Grouping*)tokens[i])->isOpen &&
                 tokens[i - 1]->tokenType == Token::TokenType::Operator && ((Operator*)tokens[i - 1])->symbol != '!')
        {
            std::cout << "Error: Malformed Input - input has an operator other than '!' directly before a grouping: '" << ((Operator*)tokens[i - 1])->symbol << ((Grouping*)tokens[i])->symbol << "'\n";
            return -1;
        }
    }

    return 0;
}

int sanitizeEquation(std::vector<Token*>& tokens)
{
    // 1) parse function names and constants (if they aren't recognized, give the user an error and return)
    //   - this includes seeing if arc comes right before a trig function or h comes right after, should probably also handle ^-1 for trig functions and convert ^n or ^(n...) to some sort of function if it is being applied to a function
    //   - will also need to throw an error if the equation ends with a function, though I might eventually support functions with no parameters (along with more than one parameter), so handling functions on a case by case basis might eventually be necessary
    for (int i = 0; i < tokens.size(); i++)
    {
        if (tokens[i]->tokenType != Token::TokenType::Function)
            continue;
        if (Function::splitFunctions(tokens, i))
            return -1;
    }

    // Ensure functions were split properly
    printParsed(tokens, "Functions Split");


    // Should probably handle trig functions around here, like combining arc and ^-1 with the trig function


    // Replace functions that should be constants with their respective values
    for (int i = 0; i < tokens.size(); i++)
    {
        if (tokens[i]->tokenType != Token::TokenType::Function)
            continue;
        long double val{Number::getConstant(((Function*)tokens[i])->name)};
        if (!val)
            continue;
        delete tokens[i];
        tokens[i] = new Number{val};
    }

    // Ensure constants were substituted in properly
    printParsed(tokens, "Constants Substituted In");

    // TODO: I need to be handling the syntax '[func]^[n]([m])' either before this function gets called, or alongside/inside of it!
    if (Grouping::addMissingFunctionGroupings(tokens))
        return -1;

    // Ensure function groupings were added properly
    printParsed(tokens, "Function Groupings Added");

    // Have to do this after inserting omitted function groupings, otherwise I would end up putting extra *'s where I don't intend them to be
    // My original reason for trying to do it before replacing functions with constants is because I was worried about consecutive numbers, but I actually check for those implicitly in the following function (it is easier to include it in the checks as a "by-product" of sorts)
    Operator::addMissingOperators(tokens);

    // Ensure *'s were added properly
    printParsed(tokens, "*'s Added");

    // Remove consectuive +'s and -'s
    // removeExtraOperators(tokens);


    // 2) add groupings following functions if they aren't there
    // 3) insert omitted *'s between numbers, groupings, and factorials ('[n!f](', ')[n!f]', ')(', 'ff', and '[n!f][n!f]' (except can't have '[n][n]'; also, if I decide to do the step about inserting groupings after functions before this, then I wouldn't need to include that in th checks, just groupings)
    // 4) convert consecutive +---+--... to one symbol
    // 5) if there is anything of the nature [*/][+-], make the [+-] apply to the following number, or if it is a function/grouping, makes some sort of sub() function in the event of a '-' and do nothing in the event of a '+'
    // 6) prepend and append missing groupings (not sure when the optimal time would be to do this; could also decouple when I find the amount to be added from when I actually do so so that they aren't in the way of the other "sanitizers")
    // 7) convert ! to fact()

    // I've been thinking; I might want each function to have its own member vector of Token*'s, though it might take around the same amount of code and complexity to do it either way (with or without functions having their own member vector)


    // Prepend and Append missing groupings
    Grouping::addMissingOuterGroupings(tokens);

    // Ensure outer Groupings were added properly
    printParsed(tokens, "Outer Groupings Added");

    // Will eventually be able to call this
    // int result = evaluate(tokens);

    return 0;
}

int evaluate(const std::vector<Token*>& tokens)
{
    // TODO: Implement this to recursively split the tokens vector up into two sides with an operator in-between
    return 0;
}
