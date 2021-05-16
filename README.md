# calc
 Command-line Calculator
***
# How to build
```
git clone https://github.com/JJCUBER/calc.git
cd calc
mkdir build
cd build
cmake ..
cmake --build .
```
#### Should be located here:
```
cd calc\Debug
calc.exe
```

<br></br>
### To build for release, replace last line of [this](#how-to-build) with:
```
cmake --build . --config Release
```
#### Should be located here:
```
cd calc\Release
calc.exe
```

<br></br>
*I believe that `cmake --build . --config Debug` would have the same effect as `cmake --build .`*

***
# \[This isn't much of an actual README at the moment]


***
# Steps (isn't 100% in order right now, so it isn't numbered)
- [x] Should convert the whole input to lowercase (to ensure functions are understood, like `ln`, `sin`, etc.).
- [ ] Input should be separated into numbers (including constants like `e` and `pi`), operators, groupings, and functions all as objects in a vector of a base type.
- [x] Should loop over parsed vector and count the number of open groupings (`(` = +1, `)` = -1, though it would be an object of type Grouping and have a bool for something like `isOpen`) and the most negative number should be the negative number of opening groupings to insert at the beginning of the vector (if `< 0`), while the end number of original open ones minus the most negative number should be the number of closing ones appended to the end (if `> 0`).  This is to ensure that the groupings are well-formed and should probably warn the user that `x` amount of opening groupings were added the beginning and `y` amount of closing groups were added to the end.
- [ ] Should loop over the vector and ensure that there are no consecutive numbers and no consecutive operators (except `!`, will have to find a way to treat it specially; might want to convert it to a function by wrapping groupings around the preceding number and putting a "fact" function before it \[or if the factorial is preceded by a grouping, just put "fact" before the whole group]).  If there are any instances of this, the program should stop short and tell the user that the input was malformed (and where).  Might want to remove consecutive opening and closing groupings, although it would probably be easier to handle this as part of the main task.  Numbers should be parsed by looking for solely `0-9` and `.`, this ensures that a malformed number like `5.9 437` or `5.9 4.64` gets caught by the "no consecutive numbers/operators" rule.
- Need to take into account special cases of valid consecutive operators, like `5*-4`, `5++4`, `5--4`, `5---4`, etc. **BEFORE** handling [bullet 5 of the malformed variants](#malformed5) for stuff like `floor(+5)`, `floor(-5)`, and `floor(---5)`.  For this, I might want to combine all consecutive `+` and `-` into the final, singular proper symbol after applying them all (like `+--+` => `+` or `+-+` => `-`; you basically just ignore the `+`'s and the `-` count determines the parity; \[`['-' count] % 2` determines odd/even]).  Then, if there is a number following and nothing preceding the chain of signs (other than the start of a grouping \[`(`]), I would want to apply the final signage to said number.  This also brings up the fact that `-(...)` is valid and I might want to make an internal `neg()` or `negative()` function for this, instead of trying to apply the `-` after the grouping is evaluated to a number.
  - [ ] Malformed Variants:
    1) [x] Starts with any operator, or ends with any operator other than `!`: `+...` or `...*` or `!...` \[**NOT** `...!`]
    2) [x] <a name="malformed2"></a>Inside grouping starts with any operator, or ends with any operator other than `!`: `(-` or `/)` or `(!` \[**NOT** `!)`]
    3) [x] Consecutive operators other than `!` followed by any other: `*+` or `+!` \[**NOT** `!+` **NOR** `!!!-`]
    4) [ ] Consecutive numbers other than constants: `# #` \[This does not apply for constants, as in `#e` or `ee`, only for actual numbers like `11985.5782 191.2`]
    5) [ ] <a name="malformed5"></a>Operator directly after function: `floor+` \[or `floor(+)`, would be caught by the [second bullet](#malformed2) though]
- <a name="ensureFunctionGrouping"></a>Should also loop over to ensure that a function is followed by an open grouping or number (if the function is followed by a number, then should add parenthesis surrounding said number directly after the function, will probably also have to deal with separation of parsing a function and parsing constants like `e` and `pi`, as currently they are parsed as a function; could probably first try to pull out an actual function \[or more] out of the parsed "function," then attempt to parse the rest as e and pi, if there remains some letters not parsed as an actual function or constant, then tell the user the input was malformed and return).
- Should also loop over and insert a multiplication operator between a number and a grouping facing away from it (`5(` and `)5`; **NOT** `5)` **NOR** `(5`), same with `!` followed by a number (`5!5` => `5!*5`).  Same with functions (`5ln(5)` => `5*ln(5)` and `ln(5)ln(5)` => `ln(5)*ln(5)`, shouldn't need to worry about functions without a group following it, as this should have been handled in [this prior step](#ensureFunctionGrouping)).
- It might be kind of confusing to properly implement functions and factorial at first, due to how they are located before/after a grouping/number; will just need to keep this in mind when separating groupings.
- Should loop backwards over the vector and split it into two groups, separated by the lowest order of operations found.  The reason behind looping backwards is because if you have multiple operations with the same precedence, you want to do the last one last, hence you separate the equation to have equate the left side of the last one first, then the right side (example: `[5+4-2+3]` => `[[5+4-2]+[3]]`).
- Should repeat splitting into two groups on each new group until a group either has nothing or has a number (the "has nothing" would be due to what was talked about earlier where there might be a group that is like "()" if it is in the original equation, like `5+()+1`, though this doesn't really make sense and I might ultimately want to the warn the user that it is malformed and have `()` or `(())` return a `0`in its position, or simply stop the program short, saying that the input is malformed.  The latter might make more sense but I'm not entirely sure at this point).
- groups in the original equation can act as a group themselves, so `[(5+4-2)+3]` => `[[5+4-2]+[3]]`
- ***I wrote all of this down in one go after thinking about it for a while without being able to write it down, so there will probably be major revisions to a lot of this.***


***
# Info about numbers and constants
- numbers (`LDBL_MIN` to `LDBL_MAX`) constants (`e`, `pi`, `phi`, \[probably won't support `i`])


***
# Order of operations (low to high)
1) `+ -`
2) `* /`
3) `^`
4) `!` \[unsure about whether ! should have higher precedence than ^, it seems like it does on WolframAlpha]
5) `%` \[same thing as above, it seems like `%` > `!` > `^` in precendence: [WolframAlpha](https://www.wolframalpha.com/input/?i=1%252%5E2%21).  Need to keep in mind that the modulus function in programming languages is a bit different from the mathematical one when it comes to negative numbers]
6) `()`
#### Functions (not entirely sure how functions should be considered in order of operations <a name="functionPrecedence"></a>\[though they seem to be prioritized compared to `!` and `%`, and most definitely not prioritized compared to `()` on [WolframAlpha](https://www.wolframalpha.com/input/?i=ln2%252%2Bln2%21%2Bln%282%21%29)]; maybe they count as a "constant," in the sense that the stuff inside of the grouping following the function becomes a "part" of the function, effectively making it a constant/"variable"?)
- `exp ln sqrt cbrt fact`
- `log` \[not sure how/if I want to implement this, as I would either need to support subscript (`_`), or have log be comma separated with the first parameter being the base.  If I do end up adding support for comma separated parameters for a function, then parsing of the "mod" function would get more confusing, as it has the forms amodb and mod(a,b) and I would have to distinguish those two apart.  Maybe I only support `%` for `a%b` syntax and only support mod for `mod(a,b)` syntax?  I'll also need to determine whether % takes everything on the left and right, like how it does in [WolframAlpha](https://www.wolframalpha.com/input/?i=5%21%2B4%252-10%2B%281%29%25%282%29)]
- \[might want to add mod, though it shouldn't really be considered a function, rather, it should be replaced by % when parsing since it has a left and right side]
- `floor ceil` \[Handling this without round errors might be a bit difficult, though the best thing to do for now would probably be to ensure that an int gets returned if the outermost part of the function is one of these (like `ceil(1*2!)`)]
- `sin cos tan csc sec cot` \[might want to add support for `arc-` prefix and `-1` exponent as inverse, if I decide to add `^-1` then it should probably get parsed as part of the function itself, though I will need to keep in mind that if it has both `arc` and `^-1`, they should cancel each other out]
- \[might want to add the hyperbolic variants of the trig functions]
- \[might want to add support for `trig_func^n(...)`]
  - \[I could kill two birds with one stone for this and `arc = ^-1` by seeing if `^` follows a function, then if the next number equals `-1` AND the function is a trig function, prepend the trig function with `arc`, or remove `arc` if it already has it, otherwise (if the `number != -1` or the function isn't a trig function) put the `^n` after the close grouping following the function (if you have something like `sin^(4*3-1)(5-4pi)`, then will need to handle this case, as in moving the whole group coming right after the `^` for the function to the end: `sin(5-4pi)^(4*3-1)`; I think that this shows that functions need to have a higher precedence than `^` (and `%`) \[further backing up what what I said [here](#functionPrecedence)], or at least when doing this I should wrap the function portion in a grouping like: `(sin(5-4pi))^(4*3-1)`)]




***
# Other/Misc
- ***This is my first time working with CMake in my own project, so hopefully I don't do anything considered a "bad practice" or flat-out incorrect***
- Since Windows already has `calc` as a command for opening the windows calculator, adding this exe to path variables will not work.  Instead, rename it to something like `jjcalc` (in the event that this is being used on Windows and you want to add it to the `path` \[`sys env`] variable).

