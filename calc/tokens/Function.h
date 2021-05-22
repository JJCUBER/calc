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

    // A trie might be overkill/too much overhead, unsure as of yet - update: it seems like it is just over 2x faster with the items that I tested it with
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

        /*
        int findWord(const std::string& s, int pos, int size)
        {
            Trie* curr{this};
            int largest{-1};
            while (pos < size)
            {
                curr = curr->children[s[pos++] - 'a'];
                if (!curr)
                    break;
                if (curr->nPos != -1)
                    largest = curr->nPos;
            }
            // return curr->nPos;
            // return curr->nPos != -1 ? curr->nPos : largest;
            return largest;
        }
        */

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

        // std::vector<std::string> names{"e", "pi", "phi", "arc", "h", "sin", "cos", "tan", "csc", "sec", "cot", "floor", "ceil", "fact", "exp", "ln", "log", "sqrt", "cbrt"};
        static const std::string names[]{"e", "pi", "phi", "arc", "h", "sin", "cos", "tan", "csc", "sec", "cot", "floor", "ceil", "fact", "exp", "ln", "log", "sqrt", "cbrt"};
        std::string input{((Function*)tokens[pos])->name};

        static Trie trie{names};
        // for (int n = 0; n < std::size(names); n++)
        //     trie.insert(names[n], n, names[n].size());

        std::vector<Substr> occurrences;
        int size(input.size());
        for (int i = 0; i < size; i++)
            trie.findAllWords(input, i, size, occurrences);

        // Prints out all occurrences visually in the correct spot (by padding with spaces)
        std::cout << input << '\n';
        for (const Substr& s : occurrences)
            std::cout << (s.iPos ? std::string(s.iPos, ' ') : "") << names[s.nPos] << '\n';
        std::cout << "\n\n";

        // above code is an alternative for the big block commented out a couple lines below

        // Instead of deleting and removing, I can reuse this Function Token
        /*
        delete tokens[pos];
        tokens.erase(tokens.begin() + pos);
        */

        /*
        // Finds all occurrences of matching substrings
        // vector<tuple<int, int, int>> potential; // start pos in input, pos in names, current matching length
        // vector<pair<int, int>> occurrences; // start pos in input, pos in names
        std::vector<Substr> potential;
        std::vector<Substr> occurrences;
        for (int i = 0; i < input.size(); i++)
        {
            for (int j = 0; j < potential.size(); j++)
            {

                // if (names[std::get<1>(potential[j])][std::get<2>(potential[j])++] != input[i])
                if (names[potential[j].nPos][potential[j].len++] != input[i])
                    potential.erase(potential.begin() + j--);
                // else if (std::get<2>(potential[j]) == names[std::get<1>(potential[j])].size())
                else if (potential[j].len == names[potential[j].nPos].size())
                {
                    // occurrences.push_back(std::make_pair(std::get<0>(potential[j]), std::get<1>(potential[j])));
                    occurrences.push_back(potential[j]);
                    potential.erase(potential.begin() + j--);
                    // Was trying to move it instead; https://stackoverflow.com/a/48683251/8815826
                    // occurrences.insert(occurrences.end(), std::make_move_iterator(potential.begin() + j))
                }
                // I moved this to be above ^ so that I wouldn't need to do an extra pass over the 'potential' vector to ensure that none of them are "complete" occurrences
                // else if (names[std::get<1>(potential[j])][std::get<2>(potential[j])++] != input[i])
                    // potential.erase(potential.begin() + j--);
            }
            // for (int j = 0; j < names.size(); j++)
            for (int j = 0; j < std::size(names); j++)
            {
                if (names[j][0] == input[i])
                {
                    if (names[j].size() == 1)
                        // occurrences.push_back(std::make_pair(i, j));
                        occurrences.emplace_back(i, j);
                    else
                        // potential.push_back(std::make_tuple(i, j, 1));
                        potential.emplace_back(i, j);
                }
            }
        }

        // Prints out all occurrences visually in the correct spot (by padding with spaces)
        std::cout << input << '\n';
        // for (auto& p : occurrences)
        for (const Substr& s : occurrences)
            //     cout << (p.first ? string(p.first, ' ') : "") << names[p.second] << '\n';
            std::cout << (s.iPos ? std::string(s.iPos, ' ') : "") << names[s.nPos] << '\n';
        // Prints out incomplete occurrences at the end
        // for (auto& t : potential)
        for (const Substr& s : potential)
        {
            // auto first = std::get<0>(t), second = std::get<1>(t), third = std::get<2>(t);
            //cout << (first ? string(first, ' ') : "") << names[second].substr(0, third) << '[' << names[second].substr(third, names[second].size() - third) << ']' << '\n';
            std::cout << (s.iPos ? std::string(s.iPos, ' ') : "") << names[s.nPos].substr(0, s.len) << '[' << names[s.nPos].substr(s.len, names[s.nPos].size() - s.len) << ']' << '\n';
        }

        std::cout << "\n\n";
        */





        // Removes any overlapping substrings, opting for the largest ones
        // *** Flaw with this code is mentioned earlier! ***
        /*
        vector<pair<int, int>> final{occurrences.back()};
        for (int i = occurrences.size() - 2; i >= 0; i--)
        {
            bool shouldAdd{true};
            // while (shouldAdd && !final.empty() && occurrences[i].first <= final.back().first && occurrences[i].first + names[occurrences[i].second].size() - 1 >= final.back().first)
            // while (shouldAdd && !final.empty() &&
            //      (occurrences[i].first >= final.back().first && occurrences[i].first <= final.back().first + names[final.back().second].size() - 1 ||
            //       occurrences[i].first + names[occurrences[i].second].size() - 1 >= final.back().first && occurrences[i].first + names[occurrences[i].second].size() - 1 <= final.back().first + names[final.back().second].size() - 1))
            while (shouldAdd && !final.empty())
            {
                int currStart = occurrences[i].first, currEnd = currStart + names[occurrences[i].second].size() - 1;
                int backStart = final.back().first, backEnd = backStart + names[final.back().second].size() - 1;
                // if (!(currStart >= backStart && currStart <= backEnd || currEnd >= backStart && currEnd <= backEnd))
                if ((currStart < backStart || currStart > backEnd) && (currEnd < backStart || currEnd > backEnd))
                    break;

                // shouldAdd = false;
                shouldAdd = names[occurrences[i].second].size() > names[final.back().second].size();
                // if (names[occurrences[i].second].size() > names[final.back().second].size())
                if (shouldAdd)
                    // final.back() = occurrences[i];
                    final.pop_back();
            }
            // else
            if (shouldAdd)
                final.push_back(occurrences[i]);
        }

        cout << input << '\n';
        for (auto& p : final)
            cout << (p.first ? string(p.first, ' ') : "") << names[p.second] << '\n';
        */

        std::sort(occurrences.rbegin(), occurrences.rend(), [](const Substr& s1, const Substr& s2) { return s1.len == s2.len ? s1.iPos < s2.iPos : s1.len < s2.len; });

        // cout << input << '\n';
        // for (const Substr& s : occurrences)
        //     cout << (s.iPos ? string(s.iPos, ' ') : "") << names[s.nPos] << '\n';

        auto haveOverlap = [&](const Substr& s1, const Substr& s2)
        {
            // int currStart = s1.iPos, currEnd = currStart + names[s1.nPos].size() - 1;
            // int backStart = s2.iPos, backEnd = backStart + names[s2.nPos].size() - 1;
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

        // auto it = set.begin();
        std::cout << input << "\nNot added:\n";
        for (int i = 1; i < occurrences.size(); i++) // , ++it)
        {
            if (!orderedFunctions.insert(occurrences[i]).second)
                std::cout << (occurrences[i].iPos ? std::string(occurrences[i].iPos, ' ') : "") << names[occurrences[i].nPos] << '\n';
            /*
            auto it = set.end();
            if (it->iPos == occurrences[i].iPos && overlapLambda(*(--it), *set.end()))
                // set.erase(--set.end());
                set.erase(it);
            */
        }
        std::cout << "\nAdded:\n";
        for (const Substr& s : orderedFunctions)
            std::cout << (s.iPos ? std::string(s.iPos, ' ') : "") << names[s.nPos] << '\n';

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
