# EigenproblemEquationSystem

!if! function=hasCapability('mfem')

The EigenproblemEquationSystem is responsible for assembling the operators that will solve the eigenproblem with strong form

!equation
{H u=\lambda u} ,

where $H$ is a linear operator acting on the variable $u$ and $\lambda$ represents an eigenvalue. EigenproblemEquationSystem has methods to build the right-hand side mass kernel, the left-hand side bilinear form and kernels and apply Dirichlet boundary conditions. 

The resulting solution in the form of a vector of eigenvalues can be exported using the [MFEMEigenvaluesPostprocessor](source/mfem/vectorpostprocessors/MFEMEigenvaluesPostprocessor.md)

!if-end!

!else
!include mfem/mfem_warning.md
