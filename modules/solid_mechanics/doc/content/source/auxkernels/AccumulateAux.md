# AccumulateAux

!syntax description /AuxKernels/AccumulateAux

Multiple variables can be accumulated in the target variable.
The accumulation is performed for the entire field within the block / boundary restriction
on every execution of the auxkernel.

!alert note
The accumulation is performed on a quadrature point basis, and thus projections are automatically performed at the same time as the accumulation if the variable finite element types are different.

!syntax parameters /AuxKernels/AccumulateAux

!syntax inputs /AuxKernels/AccumulateAux

!syntax children /AuxKernels/AccumulateAux