# LinearFVBCs System

For an overview of linear MOOSE FV please see [/linear_fv_design.md].

The difference between `LinearFVBCs` and `FVBCs` is that the boundary quantities
computed by the former (boundary values, gradients, etc.) are used in routines
within linear FV kernels. This is due to the fact that boundary conditions may need to be
applied in different manners for different terms in the partial differential equation.
This means that the `LinearFVBCs` only provide objects to specify these boundary quantities,
and would not contribute to the system matrix and right hand side directly (only through kernels).

## LinearFVBCs block

FVM boundary conditions are added to simulation input files in the `LinearFVBCs` as in the example below.

!listing test/tests/linearfvkernels/advection/advection-1d.i
         block=LinearFVBCs
         caption=Example of the LinearFVBCs block in a MOOSE input file.

In this example input, an advection equation with Dirichlet boundary on the left and outflow boundary conditions on the right is solved. To understand the differences between
these two boundary conditions, let's start with the advection equation:

\begin{equation}
  \nabla \cdot (\vec{v} u) = S,
\end{equation}

with $\vec{v}$ denoting the velocity vector, $u$ the solution and $S$ a potentially space-dependent
source term. The boundary conditions on the left can be expressed as:

\begin{equation}
  u = f(\vec{r}_b),
\end{equation}

while the outflow boundary on just advects the solution value on the boundary with a
predefined velocity.

Both boundary conditions can be applied in an integral sense through the discretized
advection term on the cell adjacent to the boundary:

\begin{equation}
  \int_V_b \nabla \cdot (\vec{v} u) dV approx \left(\sum_i \vec{n}_i \cdot \vec{v}_i u_{f,i}|S_i|\right) + \vec{n}_b \cdot \vec{v}_b u_b |S_b|~,
\end{equation}

where the $i$ index denotes internal faces of the cell, while $b$ denotes the only face on the boundary.
This means that the only thing we need to supply to this formula is a way to compute the contributions to
the system matrix and right hand side from the boundary value $u_b$. For example for the Dirichlet boundary
$u_b = f(\vec{r}_b)$, while for the outflow boundary it can be either the the cell centroid value ($u_b = u_C$)
or an extrapolated value. While the dirichlet boundary contributes to the right hand side of the system only,
the outflow boundary condition can contribute to both.

## Functions to override:

Different linear finite volume kernels might use the quantities provided by these boundary
conditions differently, but there are some common functionalities which are used more
frequently between these kernels. The following functions represent this common functionality:

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
         caption=Example source code for `LinearFVFunctorDirichletBC`.

!syntax list /LinearFVBCs objects=True actions=False subsystems=False
