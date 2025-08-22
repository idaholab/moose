# ReactionForceAux

The value of a tagged vector for a given variable is coupled to
the current AuxVariable. ReactionForceAux return the unscaled coupled DOF value with units consistent with the simulation.
AuxVariable then is written out in an exodus file.

!warning
This AuxKernel assumes that the coupled tagged vector is affected by variable scaling. This assumption holds for the residual vector, but does not for the solution vector.

!syntax parameters /AuxKernels/ReactionForceAux

!syntax inputs /AuxKernels/ReactionForceAux

!syntax children /AuxKernels/ReactionForceAux
