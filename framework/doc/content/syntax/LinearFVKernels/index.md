# LinearFVKernels System

For an overview of the linear MOOSE FV please see [/linear_fv_design.md].

For the finite volume method (FVM) when used without Newton's method, `LinearFVKernel` is the base class for `LinearFVFluxKernel` and `LinearFVElementalKernel`. These specialized objects satisfy the following tasks:

* `LinearFVFluxKernel` adds contributions to system matrices and right hand sides coming from flux terms over
  the faces between cells and boundaries. Diffusion and advection terms in PDEs serve as good examples
  for these kernels.

* `LinearFVElementalKernel` adds contributions to system matrices and right hand sides from volumetric integrals.
  Volumetric source terms or reaction terms serve as good examples for these kernels.

## LinearFVKernels block

FVM kernels which contribute to systems that are not solved via Newton's method
are added to simulation input files in the `LinearFVKernels` block.  The
`LinearFVKernels` block in the example below sets up a steady-state diffusion problem
defined by the equation:

\begin{equation}
  - \nabla \cdot D \nabla u = S.
\end{equation}

The diffusion term is represented by the kernel named `diff`.

!listing test/tests/fixmee/fv_simple_diffusion/transient.i
         block=LinearFVKernels
         id=first_linear_fv_kernel_example
         caption=Example of the LinearFVKernels block in a MOOSE input file.

The `LinearFVSourceKernel` in the example derives from `LinearFVElementalKernel` so it's a
volumetric contribution to the right hand side, while the `LinearFVDiffusionKernel` is an
`LinearFVFluxKernel` and it's a face contribution to the system matrix and right hand side.
The remaining MOOSE syntax is what you would expect to see in finite element kernel objects.
The `variable` parameter refers to the variable that this kernel is acting on (i.e. into
which equation do the contributions of this term go). This must be a linear finite-volume
variable in this case.

Boundary conditions are not discussed in these examples. Look at
[syntax files](syntax/LinearFVBCs/index.md) for details about boundary conditions.

!syntax list /LinearFVKernels objects=True actions=False subsystems=False
