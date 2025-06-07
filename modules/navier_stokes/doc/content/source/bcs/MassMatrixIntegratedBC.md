# MassMatrixIntegratedBC

This class adds the exterior boundary mass for facet unknowns. It should be used in conjunction with a mass matrix object operating on interior faces (such as [MassMatrixDGKernel.md] if not performing static condensation) which computes the facet mass for interior faces.

!syntax parameters /BCs/MassMatrixIntegratedBC

!syntax inputs /BCs/MassMatrixIntegratedBC

!syntax children /BCs/MassMatrixIntegratedBC
