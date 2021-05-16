#include <iostream>
#include <vector>

#include "tokens/Token.h"
#include "tokens/Number.h"
#include "tokens/Operator.h"
#include "tokens/Grouping.h"
#include "tokens/Function.h"

std::string getInput(int argc, char* argv[]);
void initialParse(std::string& equation, std::vector<Token*>& tokens);
void printParsed(const std::vector<Token*>& tokens);
int checkForMalformedInput(const std::vector<Token*>& tokens);
int sanitizeEquation(std::vector<Token*>& tokens);

int main(int argc, char* argv[])
{
    // [Get input]

    std::string equation = getInput(argc, argv);
    std::cout << equation << "\n";

    // Ensure there is an input/equation
    if (equation.empty())
    {
        std::cout << "Missing equation - proper format: 'calc \"{equation}\"'";
        return 0;
    }


    // [Initial parse pass-through]

    std::vector<Token*> tokens;
    initialParse(equation, tokens);

    // Printing to ensure that parsing is being done correctly
    printParsed(tokens);

    // Should be done after initial checks; I can't see any cases currently which would pass the checks when they shouldn't by doing this after (something like '(1+' would still be invalid without inserting the ')', since the equation would end with an operator other than '!')
    /*
    // Get number of groupings, aka parenthesis to prepend and append
    int min{}, ct{};
    for (Token* t : tokens)
    {
        if (t->tokenType != Token::TokenType::Grouping)
            continue;
        ct += ((Grouping*)t)->isOpen ? 1 : -1;
        min = std::min(ct, min);
    }

    // Prepend and append proper groupings if needed
    if (min < 0)
    {
        for (int i = 0; i < -min; i++)
            tokens.insert(tokens.begin(), new Grouping('('));
        std::cout << "Warning: prepended " << -min << " '('\n";
    }
    if (ct - min > 0)
    {
        for (int i = 0; i < ct - min; i++)
            tokens.push_back(new Grouping(')'));
        std::cout << "Warning: appended " << ct - min << " ')'\n";
    }

    // Printing to ensure groupings were added correctly
    printParsed(tokens);
    */


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

void printParsed(const std::vector<Token*>& tokens)
{
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
    std::cout << parsed << "\n";
}

int checkForMalformedInput(const std::vector<Token*>& tokens)
{
    // Starting with any operator
    if (tokens[0]->tokenType == Token::TokenType::Operator)
    {
        std::cout << "Error: Malformed Input - input starts with the operator (or unknown symbol) '" << ((Operator*)tokens[0])->symbol << '\'';
        return -1;
    }

    // Ending with any operator other than '!'
    if (tokens.back()->tokenType == Token::TokenType::Operator)
    {
        Operator* op = (Operator*)tokens.back();
        if (op->symbol != '!')
        {
            std::cout << "Error: Malformed Input - input ends with the operator (or unknown symbol) '" << op->symbol << '\'';
            return -1;
        }
    }

    for (int i = 1; i < tokens.size(); i++)
    {
        // Consecutive numbers
        if (tokens[i]->tokenType == Token::TokenType::Number && tokens[i - 1]->tokenType == Token::TokenType::Number)
        {
            std::cout << "Error: Malformed Input - input has consecutive numbers: '" << ((Number*)tokens[i - 1])->n << "' '" << ((Number*)tokens[i])->n << '\'';
            return -1;
        }

        // Might want to add handling of consecutive operators here for stuff like '5*-4,' '5++4,' '5--4,' '5---4,' etc. (might want to warn about extra +'s and -'s)
        // Will also need to handle functions after taking into account ^, as in 'floor(+5),' 'floor(-5),' and 'floor(---5)' should all be valid, but not 'floor(*5)'

        // Conescutive operators other than ![_] or [_][+-]
        if (tokens[i - 1]->tokenType == Token::TokenType::Operator && tokens[i]->tokenType == Token::TokenType::Operator)
        {
            Operator *prevOp = (Operator*)tokens[i - 1], *currOp = (Operator*)tokens[i];
            // if(prevOp->orderOfOp == 1 && currOp->orderOfOp != 1)
            if(prevOp->symbol != '!' && currOp->orderOfOp != 1)
            {
                std::cout << "Error: Malformed Input - input has consecutive operators other than '![_]' or '[_][+-]': '" << prevOp->symbol << currOp->symbol << '\'';
                return -1;
            }
            continue;
        }

        // Inside of group starts with any operator
        if (tokens[i - 1]->tokenType == Token::TokenType::Grouping)
        {
            Grouping* grouping = (Grouping*)tokens[i - 1];
            if (grouping->isOpen && tokens[i]->tokenType == Token::TokenType::Operator)
            {
                std::cout << "Error: Malformed Input - input has an operator directly after a grouping: '" << grouping->symbol << ((Operator*)tokens[i])->symbol << '\'';
                return -1;
            }
        }
        // Inside of group ends with any operator other than '!'
        else if (tokens[i]->tokenType == Token::TokenType::Grouping && !((Grouping*)tokens[i])->isOpen &&
                 tokens[i - 1]->tokenType == Token::TokenType::Operator && ((Operator*)tokens[i - 1])->symbol != '!')
        {
            std::cout << "Error: Malformed Input - input has an operator other than '!' directly before a grouping: '" << ((Operator*)tokens[i - 1])->symbol << ((Grouping*)tokens[i])->symbol << '\'';
            return -1;
        }
    }

    return 0;
}

int sanitizeEquation(std::vector<Token*>& tokens)
{
    // 1) parse function names and constants (if they aren't recognized, give the user an error and return)
    // 2) add groupings following functions if they aren't there
    // 3) insert omitted *'s between numbers, groupings, and factorials ('[n!f](', ')[n!f]', ')(', 'ff', and '[n!f][n!f]' (except can't have '[n][n]'; also, if I decide to do the step about inserting groupings after functions before this, then I wouldn't need to include that in th checks, just groupings)
    // 4) convert consecutive +---+--... to one symbol
    // 5) if there is anything of the nature [*/][+-], make the [+-] apply to the following number, or if it is a function/grouping, makes some sort of sub() function in the event of a '-' and do nothing in the event of a '+'
    // 6) prepend and append missing groupings (not sure when the optimal time would be to do this; could also decouple when I find the amount to be added from when I actually do so so that they aren't in the way of the other "sanitizers")

    return 0;
}

