#pragma once

#include <string>
#include <vector>
#include <utility>

#include "Token.h"

class Function : public Token
{
    struct Substr
    {
        // iPos is input pos
        // nPos + len are names pos + names len
        int iPos, nPos, len;
        Substr(int iPos, int nPos, int len) : iPos{iPos}, nPos{nPos}, len{len} {}
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
        ~Trie()
        {
            for (int i = 0; i < 26; i++)
                delete children[i];
            // std::cout << "Deleted Trie\n";
        }

        void insert(const std::string& s, int nPos, int len);
        void findAllWords(const std::string& s, int pos, int size, std::vector<Substr>& output) const;
        int remove(const std::string& s);
    };
    
public:
    std::string name;

    Function(std::string name) : Token{TokenType::Function}, name{std::move(name)} {}

    static int splitFunctions(std::vector<Token*>& tokens, int& pos);

    long double runFunc(long double n) const;

    static void testTrieRemoval();
};
