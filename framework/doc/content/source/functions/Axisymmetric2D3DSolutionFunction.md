# Axisymmetric2D3DSolutionFunction

!syntax description /Functions/Axisymmetric2D3DSolutionFunction

The 2D solution is likely to be the output of a 2D-RZ calculation, which we want to compare
to a full 3D model. This is useful for assessing the validity of the 2D-RZ geometric approximation.

The axis of symmetry for the original 2D axisymmetric calculation and for mapping
this 2D axisymmetric calculation into the 3D space can both be specified.

## Example input syntax

In this example, three `Axisymmetric2D3DSolutionFunction` are used to load results for a 2D
axisymmetric simulation in a 3D mechanics simulation. These results are then used in the `BCs`
block to impose a displacement on a boundary, and in a temperature variable using a [FunctionAux.md].

!listing modules/combined/test/tests/axisymmetric_2d3d_solution_function/3dy.i block=Functions

!syntax parameters /Functions/Axisymmetric2D3DSolutionFunction

!syntax inputs /Functions/Axisymmetric2D3DSolutionFunction

!syntax children /Functions/Axisymmetric2D3DSolutionFunction
