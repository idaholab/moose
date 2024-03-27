
# Linear Finite Volume Design Decisions in MOOSE

The main motivation for introducing a new approach for finite volume system
assembly is that many fluid dynamics solvers use a Picard-style velocity-pressure
coupling and the routines designed for computing Residuals and Jacobians for Newton
solves involve additional costs that can considerably slow down a Picard
solve which may require orders of magnitude more iterations for convergence.

To distinguish from the classical parts of MOOSE that solve nonlinear equations
implicity using the Newton (or quasi-Newton) method, the new linear class names
include a `Linear` sub-string denoting that they contribute to a [linear system
containing a matrix of coefficients and a right hand side](LinearSystem.md) instead of a
Jacobian and a residual.

## Design Choices for Finite Volume Linear System Assembly

- We assume a Picard-style iteration where the [system matrix and right hand side](LinearSystem.md)
  are assembled using the previous solution vector.
- We rely on the `LinearSystem` class in MOOSE which is a wrapper around
  the `LinearImplicit` system class of `libMesh`. The corresponding kernels will
  directly contribute to the system matrix and right hand side owned by the
  `LinearSystem` class.
- We try to keep the matrix as sparse as possible so every term that requires
  an extended stencil is treated explicitly. The assumption is that if we already use
  a Picard-style solve this will increase the total number of iterations a little bit,
  but for large runs the increased number of iterations will be offset by faster linear
  solves and assembly times.

## Linear FV Variables

Linear variables are designed only to contribute to
linear systems. The following list summarizes the main differences compared to
[finite volume variables](MooseVariableFV.md) that contribute to a Newton system:

- The linear variable does not support automatic differentiation.
- The gradient computation is not located within the variable, it is populated
  in an external loop. This will be further discussed below.
- The data members that enable quadrature-based evaluation of the variable are only used
  when interfacing with other MOOSE systems such as [UserObjects](UserObject.md),
  [AuxKernels](AuxKernel.md) or [Postprocessors](Postprocessor.md). These data members
  are not used during the solve. For this reason, the preferred avenue for interfacing
  with these variables should be the [functor system](Functors/index.md).

## Linear FV Kernels

To add contributions to the system matrix and right hand side to a linear system,
a new system has been created which corresponds to the `[LinearFVKernels]` block
in the input file. These kernels add contributions to the system matrix and right hand
side of a linear system directly.

### Flux Kernels

Similarly to the nonlinear approach with the Newton system, we often encounter
terms in the partial differential equations which contain face fluxes in
the discretized form. To represent such terms, we added a `LinearFVFluxKernel` base class which
should be used for inheritance to populate matrices and right hand sides through a
face loop. Good examples are the advection and diffusion terms in most transport
equations. Within these functions the following functions need to be overridden:

- `computeElemMatrixContribution` which computes the matrix contributions of
  the face to the degree of freedom corresponding to the element side of the face.
- `computeNeighborMatrixContribution` which computes the matrix contributions of
  the face to the degree of freedom corresponding to the neighbor side of the face.
- `computeElemRightHandSideContribution` which computes the right hand side
  contributions of the face to the degree of freedom corresponding to the element
  side of the face.
- `computeNeighborRightHandSideContribution` which computes the right hand side
  contributions of the face to the degree of freedom corresponding to the neighbor
  side of the face.
- `computeBoundaryMatrixContribution` which computes the matrix contributions of
  a boundary face to the degree of freedom corresponding to the adjacent element.
- `computeBoundaryRHSContribution` which computes the right hand side contributions of
  a boundary face to the degree of freedom corresponding to the adjacent element.

### Elemental Kernels

For volumetric effects, an element-based evaluation is necessary. Good examples are
external source terms or reaction terms in the partial differential equations.
To manage the contributions of these terms to the linear system, a
`LinearFVElementalKernel` base class has been added which can be used for inheritance.
The derived kernels need to override the following functions:

- `computeMatrixContribution` which computes the matrix contributions to the degree of
  freedom corresponding to the element.
- `computeRightHandSideContribution` which computes the right hand side contributions
  to the degree of freedom corresponding to the element.


The population of the system matrix and right hand side with face terms
(from flux kernels) is done in a face loop: `ComputeLinearFVFaceThread`.

On the other hand, the population of the system matrix and right hand side
with element-based volumetric terms is done in an element
loop: `ComputeLinearFVElementalThread`.

## Gradient Computation

Considering that the linear finite volume system is used in a Picard-style iteration,
terms that need cell gradients are lagged. Treating gradient-based terms explicitly results
in faster linear solves due to the matrix containing fewer entries. Such terms include:

- The linear terms in the extrapolated boundary conditions.
- The nonorthogonal correctors in the surface normal gradient computation.

The explicit treatment of these terms allows us to keep only one ghosted layer
(of elements shared between processes) in parallel simulations, minimizing the
linear solve time and parallel communication
time over an iteration. The setbacks of this design choice are:

- We need corrector iterations to resolve extrapolated boundary conditions
  and nonorthogonal correctors. This means that even linear systems may need to be solved
  multiple times to get a converged solution.
- It may increase the number of iterations needed to resolve nonlinearities.

## Boundary Conditions

A significant difference compared to other systems in MOOSE is the way
boundary conditions are enforced. Even though the user is still required to define the
boundary conditions in the `[LinearFVBCs]` block, these boundary conditions
are not executed on their own; instead they are used within `LinearFVFluxKernel`-s,
through the following two functions:

- `computeBoundaryMatrixContribution` which computes the matrix contributions of
  a boundary face to the degree of freedom corresponding to the adjacent element.
- `computeBoundaryRHSContribution` which computes the right hand side contributions of
  a boundary face to the degree of freedom corresponding to the adjacent element.

To create a new boundary condition the following functions need to be overridden:

- `computeBoundaryValue` computes the boundary value of the field.
- `computBoundaryNormalGradient` computes the normal gradient of the variable on this boundary.
- `computeBoundaryValueMatrixContribution` computes the matrix contribution that would come from
  the boundary value of the field, extensively used within advection kernels.
  For example, on an outflow boundary in an advection problem,
  without using linear extrapolation, one can use the cell value
  as an approximation for the boundary value: $u_b = u_C$. In this case, we can treat the outflow term
  implicitly by adding a $\vec{v} \cdot \vec{n} |S_b|$ term to the matrix which comes from
  $\vec{v} \cdot \vec{n} u_C |S_b|$ outward flux term. This function will return
  $1$ (as it is just the cell value) and the $\vec{v} \cdot \vec{n} |S_b|$ multipliers are added
  in the advection kernel.
- `computeBoundaryValueRHSContribution` computes the right hand side contributions for terms that
  need the boundary value of the field, extensively used within advection kernels.
  Using the same example as above, by employing an extrapolation to the boundary face to determine the
  boundary value, we get the following expression: $u_b = u_C+\nabla u_C d_{Cf}$, where $d_{Cf}$ is
  the vector pointing to the face center from the cell center. In this case, besides the same matrix
  contribution as above, we need to add the following term to the right hand side:
  $\vec{v} \cdot \vec{n} \nabla u_C d_{Cf} |S_b|$. Therefore, this function returns $\nabla u_C d_{Cf}$
  (as it is just the value contribution) and the other multipliers are added in the advection kernel.
- `computeBoundaryGradientMatrixContribution` computes the matrix contributions for terms that need the
  boundary gradient of the field, extensively used within diffusion kernels. Let us take a Dirichlet
  boundary condition and a diffusion kernel for example. The integral form of the diffusion term
  requires the computation of the surface normal gradient which can be approximated on an orthogonal grid as:
  \begin{equation}
    -\int\limits_{S_f}D\nabla u \vec{n}dS  \approx -D\frac{u_b - u_C}{|d_Cf|}|S_f|,
  \end{equation}
  which means that the term including $u_C$ can go to the matrix of coefficients. Therefore, this
  function will return $\frac{1}{|d_Cf|}$ with additional multipliers added at the kernel level.
- `computeBoundaryGradientRHSContribution` computes the right hand side contributions
  for terms that need the boundary gradient of the field, extensively used within diffusion kernels.
  Continuing with the example above, we add the remaining part of the expression ($-\frac{u_b}{|d_Cf|}$)
  with the opposite sign to the right hand side. The additional multipliers, e.g. the diffusion coefficient
  and the surface area, are factored in at the kernel level.




