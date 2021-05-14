﻿#include "calc.h"

void printParsed(std::vector<Token*>& tokens)
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

// inline int printError(const char* error, const char* details)
// inline int printError(std::string* error, std::string* details)
/*
template <class T, class U>
int printError(T error, U details)
{
    // TEMP
    std::cout << '\n';

    std::cout << "Error: " << error << " - " << details;
    return -1;
}
*/

int main(int argc, char* argv[])
{
    std::string equation;
    // Combines all parameters into one
    for (int i = 1; i < argc; i++)
        equation.append(argv[i]);
    for (char& c : equation)
        c = tolower(c);
    std::cout << equation << "\n";


    if (equation.empty())
    {
        std::cout << "Missing equation - proper format: 'calc \"{equation}\"'";
        return 0;
    }
    // Initial parse pass-through
    std::vector<Token*> tokens;
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

    // Printing to ensure that parsing is being done correctly
    printParsed(tokens);

    // Get number of groupings aka parenthesis to prepend and append
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


    // [Malformed Input Checks]

    // Starting with any operator
    if (tokens[0]->tokenType == Token::TokenType::Operator)
    {
        std::cout << "Error: Malformed Input - input starts with the operator '" << ((Operator*)tokens[0])->symbol << '\'';
        return -1;
    }

    // Ending with any operator other than '!'
    if (tokens.back()->tokenType == Token::TokenType::Operator)
    {
        Operator* op = (Operator*)tokens.back();
        if (op->symbol != '!')
        {
            std::cout << "Error: Malformed Input - input ends with the operator '" << op->symbol << '\'';
            return -1;
        }
    }

    for (int i = 1; i < tokens.size(); i++)
    {
        // Consecutive numbers
        if (tokens[i]->tokenType == Token::TokenType::Number && tokens[i - 1]->tokenType == Token::TokenType::Number)
        {
            std::cout << "Error: Malformed Input - input has consecutive numbers: '" << ((Number*)token[i - 1])->n << "' '" << ((Number*)token[i])->n << '\'';
            return -1;
        }

        // Might want to add handling of consecutive operators here for stuff like '5*-4,' '5++4,' '5--4,' '5---4,' etc. (might want to warn about extra +'s and -'s)
        // Will also need to handle functions after taking into account ^, as in 'floor(+5),' 'floor(-5),' and 'floor(---5)' should all be valid, but not 'floor(*5)'

        // Conescutive operators other than ![_]
        if (tokens[i - 1]->tokenType == Token::TokenType::Operator && tokens[i]->tokenType == Token::TokenType::Operator)
        {
            // cases:
            // 1) both not '!'
            // 2) only second is '!'
            // Operator *op1 = (Operator*)tokens[i - 1], *op2 = (Operator*)tokens[i];
            // if (op1->symbol != '!')
            Operator* op = (Operator*)tokens[i - 1];
            if (op->symbol != '!')
            {
                std::cout << "Error: Malformed Input - input has consecutive operators other than '![_]': '" << op->symbol << ((Operator*)tokens[i])->symbol << '\'';
                return -1;
            }
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
