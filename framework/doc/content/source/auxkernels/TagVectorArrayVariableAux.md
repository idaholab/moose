# TagVectorArrayVariableAux

Creates an auxiliary field representing the value of a tagged vector for a given
array variable (specified through the `v` parameter), and given array component
(specified through the `component` parameter). The family and order of the
auxiliary variable that this object populates must match the family and order of
the array variable specified through `v`. Note that the only allowed execute-on
option for this object is `timestep_end` since residual evaluation must be
completed before reliable indexing of the tagged vector can be performed.

!syntax description /AuxKernels/TagVectorArrayVariableAux

!syntax parameters /AuxKernels/TagVectorArrayVariableAux

!syntax inputs /AuxKernels/TagVectorArrayVariableAux

!syntax children /AuxKernels/TagVectorArrayVariableAux
