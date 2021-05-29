#include <iostream>
#include <set>
#include <algorithm>
#include <cmath>

#include "Function.h"

void Function::Trie::insert(const std::string& s, int nPos, int len)
{
    Trie* curr{this};
    for (char c : s)
    {
        Trie*& next{curr->children[c - 'a']};
        if (!next)
            next = new Trie{};
        curr = next;
    }
    curr->nPos = nPos;
    curr->len = len;
}

void Function::Trie::findAllWords(const std::string& s, int pos, int size, std::vector<Substr>& output) const
{
    int iPos{pos};
    const Trie* curr{this};
    while (pos < size)
    {
        curr = curr->children[s[pos++] - 'a'];
        if (!curr)
            break;
        if (curr->nPos != -1)
            output.emplace_back(iPos, curr->nPos, curr->len);
    }
}

// TODO: I should really fix all of this weird return -1 stuff that I have done to many of my functions.  Instead, they should return a bool (true should probably be successful, with false being failed)
int Function::Trie::remove(const std::string& s)
{
    // toRemove is a ** because I need to be able to delete and set the Trie* to nullptr, while still being able to change what toRemove points to
    Trie **toRemove{}, *curr{this}, *prev{};
    // Doesn't need to be initialized, as it isn't used until prev points to something and prev gets reassigned to point somewhere at the same time as prevOffset is set to a value every time
    int prevOffset;
    int size(s.size());
    for (int i = 0; i < size; i++)
    {
        int offset{s[i] - 'a'};
        Trie* next{curr->children[offset]};
        if (!next)
            return -1;
        if (!toRemove && prev)
            // I HAVE to get the address this way, otherwise I would be getting the address of my local pointer curr/tmp and setting that to nullptr instead of the actual child pointer
            toRemove = &prev->children[prevOffset];
        if (i != size - 1 && curr->nPos != -1)
        {
            toRemove = nullptr;
            prev = curr;
            prevOffset = offset;
            curr = next;
            continue;
        }
        for (int j = 0; j < 26; j++)
        {
            if (j == offset || !curr->children[j])
                continue;
            toRemove = nullptr;
            break;
        }

        prev = curr;
        prevOffset = offset;
        curr = next;
    }

    if (curr->nPos == -1)
        return -1;

    curr->nPos = -1;

    if (!toRemove)
        // curr->nPos = -1;
        return 0;

    for (int i = 0; i < 26; i++)
    {
        if (!curr->children[i])
            continue;
        // curr->nPos = -1;
        return 0;
    }

    delete *toRemove;
    *toRemove = nullptr;
    return 0;
}


int Function::splitFunctions(std::vector<Token*>& tokens, int& pos)
{
    // Might also want to add "a" as a shorthand for "arc"
    static const std::string names[]{"e", "pi", "phi", "arc", "a", "h", "sin", "cos", "tan", "csc", "sec", "cot", "floor", "ceil", "mod", "fact", "exp", "ln", "log", "sqrt", "cbrt"};
    // Will have to do this differently, as I will need to be able to add and remove custom constants from this trie (unless I decide to make another trie for the custom constants?  Probably not though)
    static const Trie trie{names};
    // static Trie trie{names};

    const std::string input{((Function*)tokens[pos])->name};

    std::vector<Substr> occurrences;
    int size(input.size());
    for (int i = 0; i < size; i++)
        trie.findAllWords(input, i, size, occurrences);

    if (occurrences.empty())
    {
        std::cout << "Error: Malformed Input - no functions were found in '" << input << "'\n";
        return -1;
    }

    std::sort(occurrences.rbegin(), occurrences.rend(), [](const Substr& s1, const Substr& s2) { return s1.len == s2.len ? s1.iPos < s2.iPos : s1.len < s2.len; });

    auto haveOverlap = [&](const Substr& s1, const Substr& s2)
    {
        int currStart = s1.iPos, currEnd = currStart + s1.len - 1,
            backStart = s2.iPos, backEnd = backStart + s2.len - 1;
        return currStart >= backStart && currStart <= backEnd || currEnd >= backStart && currEnd <= backEnd;
    };

    auto functionOrderComp = [&](const Substr& s1, const Substr& s2)
    {
        // Have to ensure that overlap is checked via the smaller one first because a smaller substring can potentially fit completely inside another, allowing the larger substring to not have a beginning nor end inside the smaller one ([1[23]4] => [23] is completely inside [1234] and has both ends inside it, whereas [1234] has neither end inside [23]; MUST check if the smaller one has ends inside the larger)
        return s1.iPos < s2.iPos && !(s1.len <= s2.len ? haveOverlap(s1, s2) : haveOverlap(s2, s1));
    };
    std::set<Substr, decltype(functionOrderComp)> orderedFunctions{{occurrences[0]}, functionOrderComp};

    for (int i = 1; i < occurrences.size(); i++)
        orderedFunctions.insert(occurrences[i]);

    auto printErrorFailedToSeparate = [&]()
    {
        std::cout << "Error: Malformed Input - not everything was able to be parsed into separate functions in '" << input << "'\n";
        return -1;
    };

    auto it{orderedFunctions.begin()};
    if (it->iPos)
        return printErrorFailedToSeparate();
    ((Function*)tokens[pos++])->name = input.substr(0, it->len);

    int last{it->len};
    while (++it != orderedFunctions.end())
    {
        if (it->iPos != last)
            return printErrorFailedToSeparate();
        last += it->len;

        tokens.insert(tokens.begin() + pos++, new Function{input.substr(it->iPos, it->len)});
        /* Probably want to do this separately after the fact, as I currently modify the original Function* passed in; it would be cleaner to replace functions with numbers if need-be in calc.cpp
        std::string funcName = input.substr(it->iPos, it->len);
        tokens.insert(tokens.begin() + pos++, Number::isConstant(funcName) ? (Token*)new Number{Number::getConstant(funcName)} : new Function{funcName});
        */
    }

    return last != input.size() ? printErrorFailedToSeparate() : 0;
}

long double Function::runFunc(long double n) const
{
    if (name == "sin")
        return std::sin(n);
    if (name == "cos")
        return std::cos(n);
    if (name == "tan")
        return std::tan(n);
    if (name == "csc")
        return 1.0L / std::sin(n);
    if (name == "sec")
        return 1.0L / std::cos(n);
    if (name == "cot")
        return 1.0L / std::tan(n);

    if (name == "arcsin" || name == "asin")
        return std::asin(n);
    if (name == "arccos" || name == "acos")
        return std::acos(n);
    if (name == "arctan" || name == "atan")
        return std::atan(n);
    if (name == "arccsc" || name == "acsc")
        return std::asin(1.0L / n);
    if (name == "arcsec" || name == "asec")
        return std::acos(1.0L / n);
    if (name == "arccot" || name == "acot")
        return std::atan(1.0L / n);

    if (name == "sinh")
        return std::sinh(n);
    if (name == "cosh")
        return std::cosh(n);
    if (name == "tanh")
        return std::tanh(n);
    if (name == "csch")
        return 1.0L / std::sinh(n);
    if (name == "sech")
        return 1.0L / std::cosh(n);
    if (name == "coth")
        return 1.0L / std::tanh(n);

    if (name == "arcsinh" || name == "asinh")
        return std::asinh(n);
    if (name == "arccosh" || name == "acosh")
        return std::acosh(n);
    if (name == "arctanh" || name == "atanh")
        return std::atanh(n);
    if (name == "arccsch" || name == "acsch")
        return std::asinh(1.0L / n);
    if (name == "arcsech" || name == "asech")
        return std::acosh(1.0L / n);
    if (name == "arccoth" || name == "acoth")
        return std::atanh(1.0L / n);

    if (name == "floor")
        return std::floor(n);
    if (name == "ceil")
        return std::ceil(n);

    if (name == "fact")
        // TODO: I should probably handle n being negative, along with implementing my own factorial for natural numbers (as it seems like the compiler that I am using doesn't calculate the exact integer-domain factorial even when the input is a sufficiently small integer, only some implementations do that: https://en.cppreference.com/w/cpp/numeric/math/tgamma (Notes section)
        return std::tgamma(n + 1);
    if (name == "exp")
        return std::exp(n);
    if (name == "ln")
        return std::log(n);
    if (name == "log")
        // TODO: will need to add base parsing, which should assign a value to a member of Function named base
        // return std::log(n) / std::log(base);
        return 0;
    if (name == "sqrt")
        return std::sqrt(n);
    if (name == "cbrt")
        return std::cbrt(n);

    // return 0;
}

void Function::testTrieRemoval()
{
    // static Trie t{};

    // t.insert("apple", 1, 1);
    // t.insert("app", 1, 1);
    // std::cout << t.remove("app") << '\n';
    // std::cout << t.remove("apple") << '\n';


    // t.insert("apple", 1, 1);
    // t.insert("able", 1, 1);
    // std::cout << t.remove("apple") << '\n';
    // std::cout << t.remove("able") << '\n';


    // t.insert("apple", 1, 1);
    // t.insert("banana", 1, 1);
    // std::cout << t.remove("apple") << '\n';
    // std::cout << t.remove("banana") << '\n';
    // std::cout << t.remove("ban") << '\n';
}
