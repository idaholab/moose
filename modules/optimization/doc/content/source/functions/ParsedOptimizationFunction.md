# ParsedOptimizationFunction

!syntax description /Functions/ParsedOptimizationFunction

## Overview

This function is similar to [MooseParsedFunction.md] except it has some dedicated routines relevant to optimization, particularly the ability to differentiate the function with respect to parameters. The [!param](/Functions/ParsedOptimizationFunction/expression) parameter indicates the function expression. Users may specify `x`, `y`, `z`, and `t` for space-time dependence. They may also define variables with constant values in the expression by specifying [!param](/Functions/ParsedOptimizationFunction/constant_symbol_names) and [!param](/Functions/ParsedOptimizationFunction/constant_symbol_values) pairs. Finally, parameter variables can be specified which correspond to the definitions in [!param](/Functions/ParsedOptimizationFunction/param_symbol_names). The values of these parameter variables are evaluated from the inputted vector in [!param](/Functions/ParsedOptimizationFunction/param_vector_name). The vector specified in [!param](/Functions/ParsedOptimizationFunction/param_vector_name) can either be a [vector-postprocessor](VectorPostprocessors/index.md) vector or a vector [reporter](Reporters/index.md) value. An error will occur if the length of this vector does not match the number of values specified in [!param](/Functions/ParsedOptimizationFunction/param_symbol_names).

## Example Input File Syntax

Here is an example where the parsed function represents the following expression:

!equation
f(x,y,z,t; \vec{v}) = xv_1 + yv_2^2 + zv_3^3 + tv_4^4

!listing parsed_function/parsed_function.i block=Functions

`params/vals` is the name of a vector reporter value, define as:

!listing parsed_function/parsed_function.i block=Reporters

!syntax parameters /Functions/ParsedOptimizationFunction

!syntax inputs /Functions/ParsedOptimizationFunction

!syntax children /Functions/ParsedOptimizationFunction
