# SolutionUserObject

!syntax description /UserObjects/SolutionUserObject

## Description

A solution user object reads a variable from a mesh in one simulation to another.  In
order to use a `SolutionUserObject` three additional parameters are required, an
[AuxVariable](/AuxVariables/index.md) , a [Function](/Functions/index.md) and an [AuxKernel](/AuxKernels/index.md).  The `AuxVariable` represents the
variable to be read by the solution user object.  The `SolutionUserObject` is set up to
read the old output file.  A `SolutionFunction` is required to interpolate in time and
space the data from the `SolutionUserObject`.  Finally, the `Function` is required that
will query the function and write the value into the `AuxVariable`.

## Example Input Syntax

!listing test/tests/auxkernels/solution_aux/solution_aux_exodus_interp.i block=UserObjects

!syntax parameters /UserObjects/SolutionUserObject

!syntax inputs /UserObjects/SolutionUserObject

!syntax children /UserObjects/SolutionUserObject
