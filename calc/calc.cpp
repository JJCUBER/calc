#include "calc.h"

void printParsed(std::vector<Token*>& tokens)
{
    std::string parsed;
    for(Token* t : tokens)
    {
        switch(t->tokenType)
        {
        case Token::TokenType::Number:
            parsed += std::to_string(((Number*)t)->n);
            break;
        case Token::TokenType::Operator:
            parsed.push_back(((Operator*)t)->symbol);
            break;
        case Token::TokenType::Grouping:
            parsed.push_back(((Grouping*)t)->isOpen ? '(' : ')');
            break;
        case Token::TokenType::Function:
            parsed += ((Function*)t)->name;
            break;
        }
        parsed.push_back('_');
    }
    std::cout << parsed << "\n";
}

int main(int argc, char* argv[])
{
    std::string equation;
    // Combines all parameters into one
    for (int i = 1; i < argc; i++)
        equation.append(argv[i]);
    for (char& c : equation)
        c = tolower(c);
    std::cout << equation << "\n";


    if(equation.empty())
    {
        std::cout << "Missing equation; proper format: 'calc \"{equation}\"'";
        return 0;
    }
    // Initial parse pass-through
    std::vector<Token*> tokens;
    std::string token{};
    bool isNumber{};
    for(int i = 0; i < equation.size(); i++)
    {
        if(equation[i] == '.' || isdigit(equation[i]))
        {
            if(!isNumber && !token.empty())
            {
                tokens.push_back(new Function{token});
                token.clear();
            }

            token.push_back(equation[i]);
            isNumber = true;
            continue;
        }

        if(isNumber)
        {
            tokens.push_back(new Number{std::stold(token)});
            token.clear();
        }

        isNumber = false;
        
        if (!isalpha(equation[i]))
        {
            if(!token.empty())
            {
                tokens.push_back(new Function{token});
                token.clear();
            }

            if(equation[i] != ' ')
                tokens.push_back(equation[i] == '(' || equation[i] == ')' ? (Token*)new Grouping{equation[i]} : new Operator{equation[i]});
            continue;
        }

        token.push_back(equation[i]);
    }
    if (!token.empty())
        tokens.push_back(isNumber ? (Token*)new Number{std::stold(token)} : new Function{token});

    // Printing to ensure that parsing is being done correctly
    printParsed(tokens);

    // Get number of groupings aka parenthesis to prepend and append
    int min{}, ct{};
    for(Token* t : tokens)
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
    if(ct - min > 0)
    {
        for (int i = 0; i < ct - min; i++)
            tokens.push_back(new Grouping(')'));
        std::cout << "Warning: appended " << ct - min << " ')'\n";
    }

    // Printing to ensure groupings were added correctly
    printParsed(tokens);


    return 0;
}
