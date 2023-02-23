# FVBCs System

For an overview of MOOSE FV please see [/fv_design.md].

The finite volume method (FVM) distinguishes between two types of boundary conditions.

* `FVDirichletBC` prescribes values of the FVM variables on the boundary. This boundary condition acts similarly to Dirichlet boundary conditions in FEM but it is implemented
using a ghost element method.

* `FVFluxBC` prescribes the flux on a boundary. This boundary condition is similar to
integrated boundary conditions in FEM.

Currently, the `FVDirichletBC` category only contains a single class
that applies a fixed value on the boundary. In the future, more specialized
classes will be added.

## FVBCs block

FVM boundary conditions are added to simulation input files in the `FVBCs` as in the example below.

!listing test/tests/fvkernels/fv_simple_diffusion/neumann.i
         block=FVBCs
         id=first_fv_bc_example
         caption=Example of the FVBCs block in a MOOSE input file.

In this example input, a diffusion equation with flux boundary conditions on the left and Dirichlet boundary conditions on the right is solved. To understand the differences between
these two boundary conditions, let's start with the diffusion equation:

\begin{equation}
  - \nabla \cdot D \nabla v = 0.
\end{equation}

and the boundary conditions on the left:

\begin{equation}
  - D  \nabla v \cdot \vec{n}= 5,
\end{equation}

where $\vec{n}$ is the outward normal and on the right:

\begin{equation}
  v = 42.
\end{equation}

For seeing how the flux boundary condition is applied, the diffusion equation is integrated
over the extent of an element adjacent to the left boundary and Gauss' theorem is appied to the divergence:

\begin{equation}
  -\int_{\Omega} \nabla \cdot D \nabla v dV =
  -\int_{\partial \Omega_l} D \nabla v \cdot \vec{n} dA
  -\int_{\partial \Omega \setminus \partial \Omega_l} D \nabla v \cdot \vec{n} dA
  = 5 A_{\partial \Omega_l}
  -\int_{\partial \Omega \setminus \partial \Omega_l} D \nabla v \cdot \vec{n} dA=0,
\end{equation}

where $\Omega$ is the element volume, $\partial \Omega_l$ are all faces that belong to the left sideset, $\partial \Omega$ are all faces, and $A_{\partial \Omega_l}$ is the area of face.
Flux boundary conditions are applied by replacing appropriate terms in the FVM balance by the value of the flux prescribed on the boundary.

Dirichlet boundary conditions are applied differently. Let us first write a balance equation for an element that is adjacent to the right boundary:

\begin{equation}
  -\int_{\partial \Omega_r} D \nabla v \cdot \vec{n} dA
  -\int_{\partial \Omega \setminus \partial \Omega_r} D \nabla v \cdot \vec{n} dA  =0,
\end{equation}

MOOSE uses the ghost element method to apply Dirichlet boundary conditions for FVM.
The process of using a ghost elements is the following:

1. Place a virtual element across the Dirichlet boundary.

2. Compute the value of $v$ in the ghost element as the extrapolation of the element value and the desired value on the boundary.

3. Assign the value of $v$ in the ghost element.

4. Evaluate the numerical fluxes as if you were on an interior face.

For implementing the ghost element method an extrapolation must be selected. Currently,
MOOSE FVM only supports linear extrapolation. If the value of the Dirichlet boundary condition is denoted by $v_D$ and the value in the element is denosted by $v_E$, then the ghost element value $v_G$ is:

\begin{equation}
  v_G = 2 v_D - v_E.
\end{equation}

The parameters available in boundary conditions are equivalent to [FEM boundary conditions](syntax/BCs/index.md) and are not discussed in detail here.

## FVBCs source code: FVDirichletBC

`FVDirichletBC` objects assigns a constant value on a boundary.
Implementation of a FVM Dirichlet bondary condition usually only requires overriding the
`boundaryValue` method. The `boundaryValue` method must return the value
of the variable on the Dirichlet boundary.

!listing framework/src/fvbcs/FVDirichletBC.C
         start=#include
         end=""
         id=fv_dirichlet_code
         caption=Example source code for `FVDirichletBC`.

## FVBCs source code: FVFluxBC

`FVNeumannBC` objects assign a constant flux on a boundary.
Implementation of a flux boundary condition usually only requires overriding
the `computeQpResidual()` method. `FVNeumannBC` reads a constant value from the
parameters and then returns it in `computeQpResidual()`.

!listing framework/src/fvbcs/FVNeumannBC.C
         start=#include
         end=""
         id=fv_neumann_code
         caption=Example source code for `FVNeumannBC`.

## FVBCs source code: FVBurgersOutflowBC

Flux boundary conditions can be more complicated than assigning
a constant value. In this example, the outflow contribution for
the Burgers' equation is computed. The relevant term is (note 1D):

\begin{equation}
\frac{1}{2}  \int \frac{\partial}{\partial x}v^2 dx
= \frac{1}{2}  \left(v^2_R n_R + v^2_L n_L\right),
\end{equation}

where $v_R$ and $v_L$ are the values of $v$ on the left and right
boundaries of the element and $n_R$ and $n_L$ are the outward normals on these
faces. Let's assume that the left side is a boundary face where the `FVBurgersOutflowBC` is applied. On that boundary we have $n_L = -1$.
The `FVBurgersOutflowBC` boundary condition uses upwinding, i.e. it uses the element value $v$
as boundary values on outflow faces.

The code listed below first checks if the left is actually an outflow side by using the cell value of the $v$ (element average, upwinding!) and dotting it with the normal. If $v n_L > 0$, then the left is an outflow side.
In this case the contribution $1/2 v^2 n_L$ is added, otherwise no contribution is added.

!listing test/src/fvbcs/FVBurgersOutflowBC.C
         start=FVBurgersOutflowBC::computeQpResidual()
         end=""
         id=fv_burgers_outflow_bc
         caption=Outflow boundary condition for the Burgers' equation.

In this case, the boundary condition does not represent a fixed inflow, but rather
a free outflow condition.

!syntax list /FVBCs objects=True actions=False subsystems=False
