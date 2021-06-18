#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#include "Token.h"
#include "Number.h"

class Function : public Token
{
    struct Substr
    {
        // iPos is input pos, len is name len
        int iPos, len;
        Substr(int iPos, int len) : iPos{iPos}, len{len} {}
    };

    class Trie
    {
        static void advance(Trie*& curr, int offset);
    public:
        // nPos was no longer needed and len should never be 0 if it is an "end" to a word in the trie (otherwise, I would have some invalid function names/constants; might want to use -1 instead just in case that edge case occurs though it really shouldn't?)
        int len;
        Trie* children[26];

        Trie(bool isRoot = false) : len{}, children{}
        {
            if (!isRoot)
                return;

            for (int i = 0; i < std::size(functionNames); i++)
                insert(functionNames[i]);
            //  '_' is still a variable, but the point of it is to signify that it is not in use/needed
            for (const auto& [name, _] : Number::constants)
                insert(name);

            for (const std::string& trigName : trigNames)
                insertTrigFunction(trigName);
        }

        ~Trie()
        {
            for (int i = 0; i < 26; i++)
                delete children[i];
            // std::cout << "Deleted Trie\n";
        }

        bool find(const std::string& s);
        Trie* insert(const std::string& s, bool shouldMarkAsEnd = true);
        void insertTrigFunction(const std::string& s);
        void findAllWords(const std::string& s, int pos, int size, std::vector<Substr>& output) const;
        int remove(const std::string& s);
    };
public:
    // Maybe I should name this names instead, as it is in Function::
    static const std::string functionNames[10];
    static const std::string trigNames[6];
    static Trie trie;

    std::string name;

    Function(std::string name) : Token{TokenType::Function}, name{std::move(name)} {}

    static int splitFunctions(std::vector<Token*>& tokens, int& pos);

    long double runFunc(long double n, bool& out_IsTrigFunc) const;
};
