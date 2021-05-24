#include <iostream>
#include <set>
#include <algorithm>

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


int Function::splitFunctions(std::vector<Token*>& tokens, int& pos)
{
    static const std::string names[]{"e", "pi", "phi", "arc", "h", "sin", "cos", "tan", "csc", "sec", "cot", "floor", "ceil", "fact", "exp", "ln", "log", "sqrt", "cbrt"};
    static const Trie trie{names};

    const std::string input{((Function*)tokens[pos])->name};

    std::vector<Substr> occurrences;
    int size(input.size());
    for (int i = 0; i < size; i++)
        trie.findAllWords(input, i, size, occurrences);

    if(occurrences.empty())
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
