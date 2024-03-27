# LinearFVSource

## Description

This kernel contributes to the right hand side of a system which is solved for a
linear finite volume variable [MooseLinearVariableFV.md]. The contribution for each cell
is the numerical integral of the source density function ($s$) in the following form:

!equation
\int\limits_{V_C} s dV \approx s_C V_C~,

where $s_C$ and $V_C$ denote the source density at the cell centroid and the cell volume, respectively.
This integral is added to the corresponding entry of the right hand side of the linear system.
The source density parameter ([!param](/LinearFVKernels/LinearFVSource/source_density))
accepts anything that supports functor-based evaluations. For more information on functors in
MOOSE, see [Functors/index.md].

## Example Syntax

The case below demonstrates the use of `LinearFVSource` where the force term is
supplied based upon a function form:

!listing test/tests/linearfvkernels/reaction/reaction-1d.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVSource

!syntax inputs /LinearFVKernels/LinearFVSource

!syntax children /LinearFVKernels/LinearFVSource
