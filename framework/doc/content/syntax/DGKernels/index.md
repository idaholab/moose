<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# DGKernels System

- DG means "Discontinuous Galerkin".
- As the name implies, DG methods employ discontinuous finite element basis functions.
- The finite element solution is piecewise-discontinuous across element boundaries, and continuous within each element.
- DG formulations for elliptic problems are unstable unless special "penalty" terms are employed (see, e.g. "interior penalty DG" method).
- You can use DG in MOOSE by specifying a discontinuous family of basis functions (e.g. MONOMIALS) and adding one or more `DGKernels`.
- `DGKernels` can execute alongside regular `Kernels`.
- `DGKernels` are responsible for computing residual and Jacobian contributions due to the "jump" terms along inter-element edges/faces.

## Further DGKernel documentation

!syntax list /DGKernels objects=True actions=False subsystems=False

!syntax list /DGKernels objects=False actions=False subsystems=True

!syntax list /DGKernels objects=False actions=True subsystems=False

