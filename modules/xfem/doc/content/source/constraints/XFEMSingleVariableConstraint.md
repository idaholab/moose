# XFEMSingleVariableConstraint

!syntax description /Constraints/XFEMSingleVariableConstraint

## Overview

Due to the Phantom Node method's matrix implementation requiring interaction between fragmented
element pieces to be explicitly defined, a method to specify this interaction is necessary. This
object takes in values and fluxes for a single variable and enforces these values as an ElemElem
constraint.

The variable to be constrained and the userobject defining the interface upon which the constraint
is occurring must both be specified. Next, any jumps in the variable's value or flux at the
interface are provided. Lastly, the method by which the constraint is to be enacted is defined in
the `use_penalty` parameter and a parameter dependent upon choice of enforcement method is given in
`alpha`.

Currently, there are two choices of enforcement method: Nitsche's formulation and the penalty
method. Nitsche's method has the advantage of providing consistent results but does require the
stabilization parameter (`alpha`) to be as small as possible while the method is still stable and
is only applicable to simple diffusion problems. Penalty method, while stable, does not give as
consistent results as Nitsche but can be applied to a wider variety of problems. When using penalty
method, the `alpha` parameter should be quite large (e.g. 1e6).

## Example Input File Syntax

!listing test/tests/moving_interface/verification/1D_xy_discrete2mat.i block=Constraints

!syntax parameters /Constraints/XFEMSingleVariableConstraint

!syntax inputs /Constraints/XFEMSingleVariableConstraint

!syntax children /Constraints/XFEMSingleVariableConstraint
