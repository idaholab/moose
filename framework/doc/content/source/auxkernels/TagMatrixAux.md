# TagMatrixAux

The diagonal value of the matrix (associated with a tag) is retrieved for a given node.
And the diagonal value is used as the nodal value for the AuxVariable that will be
written out in an exodus file for visualization.

Setting the [!param](/AuxKernels/TagMatrixAux/scaled) parameter to false makes
the kernel return values unaffected by variable scaling and automatic scaling.
Note, however, that nodal boundary conditions and strong constraints are
setting Jacobian entries independent of scaling and can lead to unexpected
unscaled results.

!syntax description /AuxKernels/TagMatrixAux

!syntax parameters /AuxKernels/TagMatrixAux

!syntax inputs /AuxKernels/TagMatrixAux

!syntax children /AuxKernels/TagMatrixAux

!bibtex bibliography
