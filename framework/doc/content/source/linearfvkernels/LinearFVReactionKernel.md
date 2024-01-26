# LinearFVReactionKernel

## Description

This kernel contributes to the system matrix of a system which is solved for a
linear finite volume variable [MooseLinearVariableFV.md]. The contribution for each cell
is the numerical integral of the reaction coefficient ($c(\vec{r})$) times
the field quantity ($u(\vec{r})$) in the following form:

!equation
\int\limits_V c(\vec{r})u(\vec{r})d\vec{r} \approx c(\vec{r}_C)u(\vec{r}_C) V_C~,

where $\vec{r}_C$ and $V_C$ denote the cell centroid and volume, respectively.
This means that the diagonal entry of the system matrix corresponding to cell
$C$ will be increased by $c(\vec{r}_C) V_C$.
The reaction coefficient parameter ([!param](/LinearFVKernels/LinearFVReactionKernel/coeff))
accepts anything that supports functor-based evaluations. For more information on functors in
MOOSE, see [Functors/index.md].

## Example Syntax

The case below demonstrates the use of `LinearFVReactionKernel` where the reaction coefficient is
supplied based upon a function form:

!listing test/tests/linearfvkernels/reaction/reaction-1d.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVReactionKernel

!syntax inputs /LinearFVKernels/LinearFVReactionKernel

!syntax children /LinearFVKernels/LinearFVReactionKernel

