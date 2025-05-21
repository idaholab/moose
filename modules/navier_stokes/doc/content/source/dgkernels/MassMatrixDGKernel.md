# MassMatrixDGKernel

This class can be used to compute a mass matrix for facet unknowns, e.g. for variables that live only on element faces (these are `SIDE_HIERARCHIC` finite element family variables in libMesh). This object is only executed on internal faces so in general this object should be paired with [MassMatrixIntegratedBC.md] for external faces. This class cannot be used if performing static condensation because static condensation relies on computing residuals and Jacobians on a strict per-element basis whereas DGKernels simultaneously add residuals and Jacobians to both elements on either side of the face they're currently operating on.

!syntax parameters /DGKernels/MassMatrixDGKernel

!syntax inputs /DGKernels/MassMatrixDGKernel

!syntax children /DGKernels/MassMatrixDGKernel
