# LinearFVReaction

## Description

This kernel contributes to the system matrix of a system which is solved for a
linear finite volume variable [MooseLinearVariableFV.md]. The contribution for each cell
is the numerical integral of the reaction coefficient ($c$) times
the field quantity ($u$) in the following form:

!equation
\int\limits_{V_C} c u dV \approx c_C u_C V_C~,

where $u_C$, $c_C$ and $V_C$ denote the cell centroid values of $u$ and $c$ and the volume, respectively.
This means that the diagonal entry of the system matrix corresponding to cell
$C$ will be increased by $c_C V_C$.

The reaction coefficient parameter ([!param](/LinearFVKernels/LinearFVReaction/coeff))
accepts anything that supports functor-based evaluations. For more information on functors in
MOOSE, see [Functors/index.md].

## Example Syntax

The case below demonstrates the use of `LinearFVReaction` where the reaction coefficient is
supplied based upon a function form:

!listing test/tests/linearfvkernels/reaction/reaction-1d.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVReaction

!syntax inputs /LinearFVKernels/LinearFVReaction

!syntax children /LinearFVKernels/LinearFVReaction

