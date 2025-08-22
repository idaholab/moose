# TagVectorAux

The value of a tagged vector for a given variable is coupled to
the current AuxVariable. TagVectorAux return the coupled DOF value. AuxVariable
then is written out in an exodus file.

!note
The DOF values of certain vectors, such as the residual vector, are affected by variable scaling. To obtain unscaled values with units consistent with the simulation, please use [ReactionForceAux](ReactionForceAux.md).

!syntax parameters /AuxKernels/TagVectorAux

!syntax inputs /AuxKernels/TagVectorAux

!syntax children /AuxKernels/TagVectorAux
