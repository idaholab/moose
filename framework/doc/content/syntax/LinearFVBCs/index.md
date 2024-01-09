# LinearFVBCs System

For an overview of MOOSE FV please see [/linear_fv_design.md].

The difference between `LinearFVBCs` and `FVBCs` is that the boundary quantities
computed by the former (boundary values, gradients, etc.) are used in routines
within different kernels. This is due to the fact that boundary conditions may need to be
applied in different manners for different terms in the partial differential equation.
This means that the `LinearFVBCs` only provide objects to specify these boundary quantities,
and would not contribute to the system matrix and right hand side directly (only through kernels).

## LinearFVBCs block

FVM boundary conditions are added to simulation input files in the `LinearFVBCs` as in the example below.

!listing test/tests/fvkernels/fixmee/neumann.i
         block=LinearFVFVBCs
         id=first_linear_fv_bc_example
         caption=Example of the LinearFVBCs block in a MOOSE input file.

In this example input, a diffusion equation with flux boundary conditions on the left and Dirichlet boundary conditions on the right is solved. To understand the differences between
these two boundary conditions, let's start with the diffusion equation:

\begin{equation}
  - \nabla \cdot D \nabla u = 0.
\end{equation}

and the boundary conditions on the left:

\begin{equation}
  - D  \nabla v \cdot \vec{u}= 5,
\end{equation}

where $\vec{n}$ is the outward normal and on the right:

\begin{equation}
  u = 10.
\end{equation}

For seeing how the flux boundary condition is applied, the diffusion equation is integrated
over the extent of an element adjacent to the left boundary and Gauss' theorem is applied:

\begin{equation}
  -\int\limits_{V} \nabla \cdot D \nabla u dV =
  -\int_{S_l} D \nabla u \cdot \vec{n} dS
  -\int_{S_{other}} D \nabla u \cdot \vec{n} dS
  = 5 |S_l|
  -\int_{S_{other}} D \nabla u \cdot \vec{n} dS,
\end{equation}

where $V$ is the element volume, $S_l$ are all faces that belong to the left sideset, $S_{other}$ are all faces of the lement besides the left sideset, and $|S_l|$ is the area of face.
This means that a $-5 |S_l|$ term goes to the right hand side of the linear system.

The Dirichlet boundary condition is applied differently.
Let us first write a balance equation for an element that is adjacent to the right boundary:

\begin{equation}
  -\int_{S_r} D \nabla u \cdot \vec{n} dA
  -\int_{S_{other}} D \nabla u \cdot \vec{n} dA  =0,
\end{equation}

In this case, the normal gradient $\nabla u \cdot \vec{n}$ can be expressed as described in
[LinearFVDiffusionKernel.md]. The dirichlet boundary condition is then applied through the
normal gradient by:

- Replacing the neighbor value of the variable by the dirichlet boundary value.
- Replacing the distance between cell centers by the distance of the cell center and boundary face centroid.
- Only using the cell gradient for the non-orthogonal correction term instead an interpolated gradient.

## Functions to override:

Different linear finite volume kernels might use the quantities provided by these boundary
conditions differently, but there are some common functionalities which are used more
frequently between different kernels. The following functions represent this common functionality:

- `computeBoundaryValue` computes the boundary value of the field.
- `computBoundaryNormalGradient` computes the normal gradient of the field on this boundary.
- `computeBoundaryValueMatrixContribution` computes the matrix contributions for terms that need the
  boundary value of the field, extensively used within advection kernels.
- `computeBoundaryValueRHSContribution` computes the right hand side contributions for terms that need the
  boundary value of the field, extensively used within advection kernels.
- `computeBoundaryGradientMatrixContribution` computes the matrix contributions for terms that need the
  boundary gradient of the field, extensively used within diffusion kernels.
- `computeBoundaryGradientRHSContribution` computes the right hand side contributions for terms that need the
  boundary gradient of the field, extensively used within diffusion kernels.

## LinearFVBCs source code: LinearFVFunctorDirichletBC

`LinearFVFunctorDirichletBC` object assigns a value on a boundary. This value is computed using a moose
functor. For more information on the functor system in moose, see [Functors/index.md].

!listing framework/src/fvbcs/LinearFVFunctorDirichletBC.C
         start=#include
         end=""
         id=linear_fv_functor_dirichlet_code
         caption=Example source code for `LinearFVFunctorDirichletBC`.

!syntax list /LinearFVBCs objects=True actions=False subsystems=False
