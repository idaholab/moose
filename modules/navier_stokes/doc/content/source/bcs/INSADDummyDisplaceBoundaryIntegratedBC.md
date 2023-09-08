# INSADDummyDisplaceBoundaryIntegratedBC

!syntax description /BCs/INSADDummyDisplaceBoundaryIntegratedBC

This object adds the sparsity dependence of the surface displacement degrees of
freedom on surface velocity degrees of freedom introduced by the nodal boundary
condition [INSADDisplaceBoundaryBC.md]. This sparsity must be added before
nodal boundary conditions are executed because the Jacobian matrix is assembled
prior to nodal boundary condition execution. At that time, if there is unused
sparsity in the matrix it is removed by PETSc. Hence the use of this object to
prevent new nonzero allocations during execution of `INSADDisplaceBoundaryBC`.

!syntax parameters /BCs/INSADDummyDisplaceBoundaryIntegratedBC

!syntax inputs /BCs/INSADDummyDisplaceBoundaryIntegratedBC

!syntax children /BCs/INSADDummyDisplaceBoundaryIntegratedBC
