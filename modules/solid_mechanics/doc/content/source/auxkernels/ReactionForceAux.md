# ReactionForceAux

The value of a tagged residual vector for a given variable is coupled to the current AuxVariable. ReactionForceAux return the unscaled coupled DOF value with units consistent with the simulation.

!note
This AuxKernel operates on the residual vector and always removes variable scaling from the output value.

!syntax parameters /AuxKernels/ReactionForceAux

!syntax inputs /AuxKernels/ReactionForceAux

!syntax children /AuxKernels/ReactionForceAux
