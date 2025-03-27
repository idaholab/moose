# MassMatrixDGKernel

This class can be used to compute a mass matrix for facet unknowns, e.g. for variables that live only on element faces (these are `SIDE_HIERARCHIC` finite element family variables in libMesh). This object only is executed on internal faces so in general this object should be paired with [MassMatrixIntegratedBC.md].

!syntax parameters /DGKernels/MassMatrixDGKernel

!syntax inputs /DGKernels/MassMatrixDGKernel

!syntax children /DGKernels/MassMatrixDGKernel
