# ArrayParsedAux

!syntax description /AuxKernels/ArrayParsedAux

## Overview

This auxiliary kernel is meant to emulate the behavior and syntax of [ParsedAux.md] for array variables.
Using [!param](/AuxKernels/ArrayParsedAux/expression), one can define a general parsed function dependent on array variables, scalar field variables, mesh coordinates, and time:

!equation
u_i = f(v_{1,i}, v_{2,i},..., w_1, w_2,..., x, y, z, t), \quad i=1,...,n,

where $u$ is the array variable specified in [!param](/AuxKernels/ArrayParsedAux/variable) with $n$ components, $i$ is the component index of the variable, $v$ are coupled array variables specified in [!param](/AuxKernels/ArrayParsedAux/coupled_array_variables), $w$ are coupled field variables specified in [!param](/AuxKernels/ArrayParsedAux/coupled_variables), and $x$, $y$, $z$, and $t$ are coordinates used if [!param](/AuxKernels/ArrayParsedAux/use_xyzt) is `true`.

## Example Input File Syntax

This is an example that defines the following expression:

!equation
\texttt{sum} = \sum_{i=1}^n\texttt{parsed}_i = \sum_{i=1}^n(\texttt{u}_i^2 + \texttt{v}_i)(x - \texttt{const})\pi

!listing array_parsed_aux.i block=AuxKernels

!syntax parameters /AuxKernels/ArrayParsedAux

!syntax inputs /AuxKernels/ArrayParsedAux

!syntax children /AuxKernels/ArrayParsedAux
