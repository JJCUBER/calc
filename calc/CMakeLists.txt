# CMakeList.txt : CMake project for calc, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (calc "calc.cpp" "calc.h" "tokens/Token.h" "tokens/Number.h" "tokens/Operator.h" "tokens/Grouping.h" "tokens/Function.h")

# TODO: Add tests and install targets if needed.
