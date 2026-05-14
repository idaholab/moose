# EigenproblemEquationSystem

!if! function=hasCapability('mfem')

The EigenproblemEquationSystem is responsible for assembling the operators that will solve the eigenproblem with strong form

$$A u = \lambda M u,$$

where $A$ is the stiffness matrix assembled from the problem kernels, $M$ is a mass matrix constructed by the eigensolver, and $\lambda$ represents an eigenvalue. EigenproblemEquationSystem has methods to build the right-hand side mass kernel, the left-hand side bilinear form and kernels and apply Dirichlet boundary conditions. 

The resulting solution in the form of a vector of eigenvalues can be exported using the [MFEMEigenvaluesPostprocessor](source/mfem/vectorpostprocessors/MFEMEigenvaluesPostprocessor.md)

!if-end!

!else
!include mfem/mfem_warning.md
