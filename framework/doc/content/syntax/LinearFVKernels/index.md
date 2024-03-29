# LinearFVKernels System

For the finite volume method (FVM) when used without Newton's method, `LinearFVKernel` is the base class for `LinearFVFluxKernel` and `LinearFVElementalKernel`. These specialized objects satisfy the following tasks:

* `LinearFVFluxKernel` adds contributions to system matrices and right hand sides coming from flux terms over
  the faces between cells and boundaries. Diffusion and advection terms in PDEs serve as good examples
  for these kernels.

* `LinearFVElementalKernel` adds contributions to system matrices and right hand sides from volumetric integrals.
  Volumetric source terms or reaction terms serve as good examples for these kernels.

For more information on general design choices in this setting [click here](/linear_fv_design.md)

## LinearFVKernels block

FVM kernels which contribute to systems that are not solved via Newton's method
are added to simulation input files in the `LinearFVKernels` block.  The
`LinearFVKernels` block in the example below sets up a steady-state diffusion problem
defined by the equation:

\begin{equation}
  - \nabla \cdot D \nabla u = S.
\end{equation}

The diffusion term is represented by the kernel named `diffusion`.

!listing test/tests/linearfvkernels/diffusion/diffusion-2d.i
         block=LinearFVKernels
         caption=Example of the LinearFVKernels block in a MOOSE input file.

The `LinearFVSource` in the example derives from `LinearFVElementalKernel` so it's a
volumetric contribution to the right hand side, while the `LinearFVDiffusion` is an
`LinearFVFluxKernel` and it's a face contribution to the system matrix and right hand side.
The remaining MOOSE syntax is what you would expect to see in finite element kernel objects.
The `variable` parameter refers to the variable that this kernel is acting on (i.e. into
which equation do the contributions of this term go). This must be a linear finite-volume
variable in this case.

Boundary conditions are not discussed in these examples. We recommend visiting the
[LinearFVBCs/index.md) page for details about boundary conditions.

!syntax list /LinearFVKernels objects=True actions=False subsystems=False
