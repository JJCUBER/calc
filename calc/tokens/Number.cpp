#include <numbers>

#include "Number.h"

long double Number::getConstant(const std::string& s)
{
    // It would probably be a good idea to do this in a less hardcoded way; this is meant to be temporary
    return s == "e" ? std::numbers::e : s == "pi" ? std::numbers::pi : s == "phi" ? std::numbers::phi : 0.0L;
}
