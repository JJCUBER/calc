# calc
 Command-line Calculator

***
# [This isn't an actual README at the moment]
***

Steps (isn't 100% in order right now, so it isn't numbered):
- Should convert the whole input to lowercase (to ensure functions are understood, like "ln," "sin," etc.).
- Input should be separated into numbers (including constants like "e" and "pi"), operators, groupings, and functions all as objects in a vector of a base type.
- Should loop over parsed vector and count the number of open groupings ("(" = +1, ")" = -1, though it would be an object of type Grouping and have a bool for something like "isOpen") and the most negative number should be the number of opening groupings to insert at the beginning of the vector, while the end number plus the number of opening ones inserted should be the number of closing ones appended to the end.  This is to ensure that the groupings are well-formed and should probably warn the user that x amount of opening groupings were added the beginning and y amount of closing groups were added to the end.
- Should loop over the vector and ensure that there are no consecutive numbers and no consecutive operators (except !, will have to find a way to treat it specially; might want to convert it to a function by wrapping groupings around the preceding number and putting a "fact" function before it [or if the factorial is preceded by a grouping, just put "fact" before the whole group]).  If there are any instances of this, the program should stop short and tell the user that the input was malformed (and where).  Might want to remove consecutive opening and closing groupings, although it would probably be easier to handle this as part of the main task.  Numbers should be parsed by looking for solely 0-9 and '.', this ensures that a malformed number like 5.9 437 or 5.9 4.64 gets caught by the "no consecutive numbers/operators" rule.
- Should also loop over to ensure that a function is followed by an open grouping or number (if the function is followed by a number, then should add parenthesis surrounding said number directly after the function, will probably also have to deal with separation of parsing a function and parsing constants like e and pi, as currently they are parsed as a function; could probably first try to pull out an actual function [or more] out of the parsed "function," then attempt to parse the rest as e and pi, if there remains some letters not parsed as an actual function or constant, then tell the user the input was malformed and return).
- Should also loop over and insert a multiplication operator between a number and a grouping facing away from it ("5(" and ")5", not "5)" nor "(5"), same with '!' followed by a number ("5!5" => "5!*5").
- It might be kind of confusing to properly implement functions and factorial at first, due to how they are located before/after a grouping/number; will just need to keep this in mind when separating groupings.
- Should loop backwards over the vector and split it into two groups, separated by the lowest order of operations found.  The reason behind looping backwards is because if you have multiple operations with the same precedence, you want to do the last one last, hence you separate the equation to have equate the left side of the last one first, then the right side (example: [5+4-2+3] => [[5+4-2]+[3]]).
- Should repeat splitting into two groups on each new group until a group either has nothing or has a number (the "has nothing" would be due to what was talked about earlier where there might be a group that is like "()" if it is in the original equation, like "5+()+1," though this doesn't really make sense and I might ultimately want to the warn the user that it is malformed and have "()" or "(())" return a 0 in its position, or simply stop the program short, saying that the input is malformed.  The latter might make more sense but I'm not entirely sure at this point).
- groups in the original equation can act as a group themselves, so "[(5+4-2)+3]" => "[[5+4-2]+[3]]"
- *** I wrote all of this down in one go after thinking about it for a while without being able to write it down, so there will probably be major revisions to a lot of this. ***



Info about numbers and constants:
- numbers (-inf, inf) constants (e, pi, golden ratio [unsure how I should abbreviate this, maybe "golden" and/or "gr?"], [probably won't support 'i'])



Order of operations (low to high):
+ -
* /
^
! [unsure about whether ! should have higher precedence than ^, it seems like it does on WolframAlpha]
% [same thing as above, it seems like % > ! > ^ in precendence: https://www.wolframalpha.com/input/?i=1%252%5E2%21 .  Need to keep in mind that the modulus function in programming languages is a bit different from the mathematical one when it comes to negative numbers]
()
Functions (not entirely sure how functions should be considered in order of operations; maybe they count as a "constant," in the sense that the stuff inside of the grouping following the function becomes a "part" of the function, effectively making it a constant/"variable"?):
exp ln sqrt cbrt fact
log [not sure how/if I want to implement this, as I would either need to support subscript ("_"), or have log be comma separated with the first parameter being the base.  If I do end up adding support for comma separated parameters for a function, then parsing of the "mod" function would get more confusing, as it has the forms amodb and mod(a,b) and I would have to distinguish those two apart.  Maybe I only support % for a%b syntax and only support mod for mod(a,b) syntax?]
[might want to add mod, though it shouldn't really be considered a function, rather, it should be replaced by % when parsing since it has a left and right side]
floor ceil [Handling this without round errors might be a bit difficult, though the best thing to do for now would probably be to ensure that an int gets returned if the outermost part of the function is one of these (like "ceil(1*2!)")]
sin cos tan csc sec cot [might want to add support for "arc-" prefix and "-1" exponent as inverse, if I decide to add ^-1 then it should probably get parsed as part of the function itself, though I will need to keep in mind that if it has both arc and ^-1, they should cancel each other out]
[might want to add the hyperbolic variants of the trig functions]






Other/Misc:
- *** This is my first time working with CMake in my own project, so hopefully I don't do anything considered a "bad practice" or flat-out incorrect
- Since Windows already has "calc" as a command for opening the windows calculator, adding this exe to path variables will not work.  Instead, rename it to something like jjcalc (in the event that this is being used on Windows and you want to add it to the path [sys env] variable).
