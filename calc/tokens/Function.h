#pragma once

#include <string>
#include <utility>

#include "Token.h"



#include <iostream>
#include <vector>
#include <set>
#include <algorithm>



class Function : public Token
{
    struct Substr
    {
        // iPos is input pos
        // nPos + len are names pos + names len
        int iPos, nPos, len;
        Substr(int iPos, int nPos, int len = 1) : iPos{iPos}, nPos{nPos}, len{len} {}
    };

    struct Trie
    {
        int nPos, len;
        Trie* children[26];

        Trie() : nPos{-1}, len{-1}, children{} {}
        template<size_t N>
        Trie(const std::string (&names)[N]) : Trie{}
        {
            for (int n = 0; n < N; n++)
                insert(names[n], n, names[n].size());
        }

        void insert(const std::string& s, int nPos, int len)
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

        void findAllWords(const std::string& s, int pos, int size, std::vector<Substr>& output)
        {
            int iPos{pos};
            Trie* curr{this};
            while (pos < size)
            {
                curr = curr->children[s[pos++] - 'a'];
                if (!curr)
                    break;
                if (curr->nPos != -1)
                    output.emplace_back(iPos, curr->nPos, curr->len);
            }
        }
    };
    
public:
    std::string name;

    Function(std::string name) : Token{TokenType::Function}, name{std::move(name)} {}

    static int splitFunctions(std::vector<Token*>& tokens, int& pos)
    {

        // vector<string> names{"apple", "e", "banana", "extra", "app", "leap"};
        // string input{"appleappleappeextraapplesbananabananasapple"};
        // vector<string> names{"e", "ceil", "floor"};
        // string input{"eceileeeeeeeeceilflooreee"};
        // vector<string> names{"zapa", "pap"};
        // string input{"zapapapp"};
        // vector<string> names{"apaz", "pap"};
        // string input{"ppapapaz"};
        // vector<string> names{"zapa", "pap"};
        // string input{"zapapapp"};
        // Unfortunately, my code for removing overlapping substrings doesn't really work, in the sense that there are edge cases like this one where some substrings that can still fit get removed :/
        // vector<string> names{"abcd", "cde", "ef"};
        // string input{"abcdef"};
        // std::vector<std::string> names{"e", "ceil", "floor"};
        // std::string input{"eceileeeeeeeeceilcflooreee"};

        static const std::string names[]{"e", "pi", "phi", "arc", "h", "sin", "cos", "tan", "csc", "sec", "cot", "floor", "ceil", "fact", "exp", "ln", "log", "sqrt", "cbrt"};
        std::string input{((Function*)tokens[pos])->name};

        static Trie trie{names};

        std::vector<Substr> occurrences;
        int size(input.size());
        for (int i = 0; i < size; i++)
            trie.findAllWords(input, i, size, occurrences);

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

        for (int i = 1; i < occurrences.size(); i++) // , ++it)
            orderedFunctions.insert(occurrences[i]);

        auto it{orderedFunctions.begin()};
        if (it->iPos)
        {
            std::cout << "Not everything was able to be parsed into separate functions\n";
            return -1;
        }
        ((Function*)tokens[pos++])->name = input.substr(0, it->len);

        int last{it->len};
        while (++it != orderedFunctions.end())
        {
            if (it->iPos != last)
            {
                std::cout << "Not everything was able to be parsed into separate functions\n"; // << last << "!=" << it->iPos;
                return -1;
            }
            last += it->len;

            tokens.insert(tokens.begin() + pos++, (Token*)new Function{input.substr(it->iPos, it->len)});
            /* Probably want to do this separately after the fact, as I currently modify the original Function* passed in; it would be cleaner to replace functions with numbers if need-be in calc.cpp
            std::string funcName = input.substr(it->iPos, it->len);
            tokens.insert(tokens.begin() + pos++, Number::isConstant(funcName) ? (Token*)new Number{Number::getConstant(funcName)} : new Function{funcName});
            */
        }

        return 0;
    }
};
