# XFEMSingleVariableConstraint

!syntax description /Constraints/XFEMSingleVariableConstraint

## Overview

Without any other intervention, the phantom node method used by MOOSE's XFEM system results in
Heaviside enrichment of all solution fields at the locations of interfaces, with zero flux
conditions. In many cases, there is a need to impose constraints across the interface on one or more
of the solutions fields in a multiphysics XFEM simulation, which is the purpose of
`XFEMSingleVariableConstraint`. The object takes in jumps in solutions and fluxes for a single
variable and enforces prescribed jumps as a constraint between element fragments.

The variable to be constrained and the UserObject defining the interface upon which the constraint
is occurring must both be specified. Next, any jumps in the variable's value or flux at the
interface are provided. Lastly, the method by which the constraint is to be enacted is defined in
the `use_penalty` parameter and a parameter dependent upon choice of enforcement method is given in
`alpha`.

Currently, there are two choices of enforcement method: Nitsche's formulation and the penalty
method. Nitsche's method has the advantage of providing consistent results but does require the
stabilization parameter (`alpha`) to be as small as possible to maintain solution stability and is
only applicable to simple diffusion problems. The penalty method, while stable, does not give
results as consistent as Nitsche but can be applied to a wider variety of problems. When using the
penalty method, the `alpha` parameter should be quite large relative to the Jacobian entries for
the physics to which the penalty method is being applied.

## Example Input File Syntax

!listing test/tests/moving_interface/verification/1D_xy_discrete2mat.i block=Constraints

!syntax parameters /Constraints/XFEMSingleVariableConstraint

!syntax inputs /Constraints/XFEMSingleVariableConstraint

!syntax children /Constraints/XFEMSingleVariableConstraint
