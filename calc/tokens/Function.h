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
        //  I'm not really planning on ever using the Trie anywhere else other than my single static declaration (and its children), so there is no point in having a destructor; however, if I ever do decide to use it elsewhere, I should probably create a destructor which deletes each child

        void insert(const std::string& s, int nPos, int len);
        void findAllWords(const std::string& s, int pos, int size, std::vector<Substr>& output) const;
    };
    
public:
    std::string name;

    Function(std::string name) : Token{TokenType::Function}, name{std::move(name)} {}

    static int splitFunctions(std::vector<Token*>& tokens, int& pos);
};
