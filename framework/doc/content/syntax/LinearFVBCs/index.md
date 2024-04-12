# LinearFVBCs System

The difference between `LinearFVBCs` and `FVBCs` is that the boundary quantities
computed by the former (boundary values, gradients, etc.) are used in routines
within linear FV kernels. This is due to the fact that boundary conditions may need to be
applied in different manners for different terms in the partial differential equation.
This means that the `LinearFVBCs` only provide objects to specify these boundary quantities,
and would not contribute to the system matrix and right hand side directly (only through kernels).

For more information on general design choices in this setting [click here](/linear_fv_design.md)

## LinearFVBCs block

FVM boundary conditions are added to simulation input files in the `LinearFVBCs` as in the example below.

!listing test/tests/linearfvkernels/advection/advection-1d.i
         block=LinearFVBCs
         caption=Example of the LinearFVBCs block in a MOOSE input file.

In this example input, an advection equation with Dirichlet boundary condition on the left
and outflow boundary conditions on the right is solved. To understand the differences between
these two boundary conditions, let's start with the advection equation:

\begin{equation}
  \nabla \cdot (\vec{v} u) = S,
\end{equation}

with $\vec{v}$ denoting the velocity vector, $u$ the solution, and $S$ a potentially space-dependent
source term. The boundary condition on the left can be expressed as:

\begin{equation}
  u_b = f(x_b),
\end{equation}

while the outflow boundary expresses outward advection with the solution value
on the boundary and a predefined velocity.

Both boundary conditions can be applied in an integral sense through the discretized
advection term on the cell adjacent to the boundary:

\begin{equation}
  \int\limits_{V_b} \nabla \cdot (\vec{v} u) dV \approx \left(\sum\limits_i \vec{n}_i
  \cdot \vec{v}_i u_{f,i}|S_i|\right) + \vec{n}_b \cdot \vec{v}_b u_b |S_b|~,
\end{equation}

where the $i$ index denotes internal faces of the cell, while $b$ denotes the only face on the boundary.
This means that the only thing we need to supply to this formula is a way to compute the contributions to
the system matrix and right hand side from the boundary value $u_b$.
For example for the Dirichlet boundary $u_b = f(x)$,
while for the outflow boundary it can be either the cell centroid value ($u_b = u_C$)
or an extrapolated value. This also means that the Dirichlet boundary contributes to
the right hand side of the system only, whereas the outflow boundary condition can contribute to both.

## Functions to override:

Different linear finite volume kernels might use the quantities provided by these boundary
conditions differently, but these APIs should be implemented for boundary conditions of linear systems:

- `computeBoundaryValue` computes the boundary value of the field.
- `computeBoundaryNormalGradient` computes the normal gradient of the variable on this boundary.

For derived classes of linear system boundary conditions, we recommend following the same design pattern as the
`LinearAdvectionDiffusionBC` parent class.
For all boundary conditions (Neumann and Dirichlet) for an advection-diffusion problem,
we implemented the following four APIs:

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
  Using the same example as above, the remaining part of the expression belongs to the right hand side
  meaning that a $\frac{u_b}{|d_Cf|}$ term will be added with additional multipliers
  applied at the kernel level.

## LinearFVBCs source code: LinearFVAdvectionDiffusionFunctorDirichletBC

`LinearFVAdvectionDiffusionFunctorDirichletBC` object assigns a value on a boundary. This value is computed using a moose
functor. For more information on the functor system in moose, see [Functors/index.md].

!listing framework/src/linearfvbcs/LinearFVAdvectionDiffusionFunctorDirichletBC.C
         start=#include
         end=""
         caption=Example source code for `LinearFVAdvectionDiffusionFunctorDirichletBC`.

!syntax list /LinearFVBCs objects=True actions=False subsystems=False
