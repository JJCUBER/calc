#include <iostream>
#include <set>
#include <algorithm>
#include <cmath>

#include "Function.h"
#include "Number.h"

// const std::string Function::functionNames[]{"arc", "a", "h", "sin", "cos", "tan", "csc", "sec", "cot", "floor", "ceil", "mod", "fact", "exp", "ln", "log", "sqrt", "cbrt", "neg"};
const std::string Function::functionNames[10]{"floor", "ceil", "mod", "fact", "exp", "ln", "log", "sqrt", "cbrt", "neg"};
const std::string Function::trigNames[6]{"sin", "cos", "tan", "csc", "sec", "cot"};
Function::Trie Function::trie{true};


void Function::Trie::advance(Trie*& curr, int offset)
{
    Trie*& next{curr->children[offset]};
    if (!next)
        next = new Trie{};
    curr = next;
}

bool Function::Trie::find(const std::string& s)
{
    Trie* curr{this};
    for (char c : s)
    {
        curr = curr->children[c - 'a'];
        if (!curr)
            return false;
    }
    return curr->len;
}

Function::Trie* Function::Trie::insert(const std::string& s, bool shouldMarkAsEnd)
{
    Trie* curr{this};
    for (char c : s)
        advance(curr, c - 'a');
    if(shouldMarkAsEnd)
        curr->len = s.size();
    return curr;
}

// Inserts all variants of a trig function concurrently, preventing iterating over parts multiple times, almost like unrolling a for loop in some aspects (ex [sin]: sin, asin, arcsin, sinh, asinh, arcsinh)
// Adding all variants of each trig function does make the trie larger and slower to traverse, but simplifies many aspects of the code tremendously, making it potentially just as fast
void Function::Trie::insertTrigFunction(const std::string& s)
{
    int len(s.size());
    Trie *base{this}, *a{insert("a", false)}, *arc{insert("arc", false)};

    auto groupAdvance = [&](char c)
    {
        int offset{c - 'a'};
        advance(base, offset);
        advance(a, offset);
        advance(arc, offset);
    };

    auto assign = [&]()
    {
        base->len = len;
        a->len = len + 1;
        arc->len = len + 3;
    };

    for (char c : s)
        groupAdvance(c);
    assign();

    groupAdvance('h');
    len++;
    assign();
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
        if (curr->len)
            output.emplace_back(iPos, curr->len);
    }
}

bool Function::Trie::remove(const std::string& s)
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
            return false;
        if (!toRemove && prev)
            // I HAVE to get the address this way, otherwise I would be getting the address of my local pointer curr/tmp and setting that to nullptr instead of the actual child pointer
            toRemove = &prev->children[prevOffset];
        if (i != size - 1 && curr->len)
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

    if (!curr->len)
        return false;

    curr->len = 0;

    if (!toRemove)
        return true;

    for (int i = 0; i < 26; i++)
    {
        if (!curr->children[i])
            continue;
        return true;
    }

    delete *toRemove;
    *toRemove = nullptr;
    return true;
}


bool Function::splitFunctions(std::vector<Token*>& tokens, int& pos)
{
    const std::string input{((Function*)tokens[pos])->name};

    std::vector<Substr> occurrences;
    int size(input.size());
    for (int i = 0; i < size; i++)
        trie.findAllWords(input, i, size, occurrences);

    if (occurrences.empty())
    {
        std::cout << "Error: Malformed Input - no functions were found in '" << input << "'\n";
        return false;
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
        return false;
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
    }

    return last != input.size() ? printErrorFailedToSeparate() : true;
}

long double Function::runFunc(long double n, bool& out_IsTrigFunc) const
{
    out_IsTrigFunc = true;
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

    out_IsTrigFunc = false;

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

    if (name == "neg")
        return -n;

    // return 0;
}
