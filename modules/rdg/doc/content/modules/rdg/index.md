# rDG Module

The MOOSE rDG module is a library for the implementation of simulation tools that solve
convection-dominated problems using the class of so-called reconstructed discontinuous Galerkin (rDG)
methods. The specific rDG method implemented in this module is rDG(P0P1), which is equivalent to the
second-order cell-centered finite volume method (FVM). Cell-centered FVMs are regarded as a subset of
rDG methods in the case when the baseline polynomial solution in each element is a constant
monomial. The FVMs are the most widely used numerical methods in areas such as computational fluid
dynamics (CFD) and heat transfer, computational acoustics, and magnetohydrodynamics (MHD). This
module provides a systematic solution for implementing all required components in a second-order FVM
such as slope reconstruction, slope limiting, numerical flux, and proper boundary conditions.

!alert note
To avoid ambiguity, the term "rDG" should be understood as "cell-centered FVM" in this module, and
these two terms are used interchangeably in the following text.

## The Advection Equation

To best help developers and/or users better understand the rDG method so that they can try to develop
new applications or use existing rDG-based codes, we use the advection equation (the simplest
convection-dominated system that we can think of) as an example to describe the implementation of
cell-centered FVM in MOOSE.

The advection equation for a conserved quantity described by a scalar field $\psi$ is expressed
mathematically by a continuity equation as

\begin{equation}
\label{eq:advection_equation}
\frac{\partial \psi}{\partial t} + \nabla\cdot(\psi{\bf v}) = 0,
\end{equation}
where $\nabla\cdot$ is the divergence operator, and ${\bf v}$ is the velocity field. If the flow is assumed to be incompressible, that is, the velocity field satisfies
\begin{equation}
\nabla\cdot{\bf v} = 0,
\end{equation}
the above equation can be rewritten as

\begin{equation}
\frac{\partial \psi}{\partial t} + {\bf v}\cdot\nabla(\psi) = 0.
\end{equation}

## Finite Volume Method

[eq:advection_equation] can be discretized in space using a cell-centered FVM. In an FVM,
the computational domain $\Omega$ is divided by a set of non-overlapping control volumes $\Omega_i$,
which can be one or a combination of the most common element types, e.g. line segment in 1D,
triangles and quadrilaterals in 2D, and tetrahedra, prisms, pyramids, and hexahedra in 3D. On each
control volume, the integral form of the governing equations is required to be satisfied,
\begin{equation}
\label{eq:fvm_integral}
  \int_{\Omega_i}\frac{\partial\psi}{\partial t}~{\rm d}V
+ \int_{\Omega_i}\nabla\cdot(\psi{\bf v})~{\rm d}V
= 0.
\end{equation}
The cell-averaged conservative variable, $\psi_i$, is taken to be the unknown and defined by
\begin{equation}
\psi_i = \frac{1}{V_i}\int_{\Omega_i}\psi~{\rm d}V
\end{equation}
where $V_i$ is the volume of the control volume $\Omega_i$. The following equation can then be
derived using the divergence theorem,
\begin{equation}
\label{eq:fvm_divergence_theorem_bc}
  V_i\frac{{\rm d}\psi_i}{{\rm d}t}
+ \sum_{j \in N_i}\int_{\Gamma_{ij}}(\psi_{ij}{\bf v}_{ij})\cdot{\bf n}_{ij}~{\rm d}S
+ \sum_{\Gamma_{ib} \in \partial\Omega}\int_{\Gamma_{ib}}(\psi_{ib}{\bf v}_{ib})\cdot{\bf n}_{ib}~{\rm d}S
= 0,
\end{equation}
where $\Gamma_{ij}=\partial\Omega_i\cap\partial\Omega_j$ denotes an interior common face between cell
$\Omega_i$ and $\Omega_j$, $\Gamma_{ib}=\partial\Omega_i\cap\partial\Omega$ denotes a face on the
boundary of domain $\Omega$; and ${\bf n}_{ij}$ and ${\bf n}_{ib}$ are the unit vectors normal to
face $\Gamma_{ij}$ and $\Gamma_{ib}$, respectively.  For each cell $\Omega_i$, $N_i$ represents a set
of neighboring cells, $\Omega_j$, sharing a common face, $\Gamma_{ij}$.

Because the numerical solution is discontinuous between cell interfaces, the interface fluxes are not
uniquely defined.  The flux, $(\psi_{ij}{\bf v}_{ij})\cdot{\bf n}_{ij}$, appearing in the second term
of [eq:fvm_divergence_theorem_bc] should be replaced by a numerical flux function
$\mathcal{H}(\psi_{ij}, \psi_{ji}, {\bf n}_{ij})$, i.e.,
\begin{equation}
\mathcal{H}(\psi_{ij}, \psi_{ji}, {\bf n}_{ij}) \approx (\psi_{ij}{\bf v}_{ij})\cdot{\bf n}_{ij},
\end{equation}
where $\psi_{ij}$ and $\psi_{ji}$ are the conservative variable at the "left" and "right" side of the
cell interface ($i < j$).  In the case of first-order FVM, the solution in each cell is assumed to be
constant in space.  Then on any interior face, $\Gamma_{ij}$, the two states are simply $\psi_{ij} =
\psi_i$ and $\psi_{ji} = \psi_j$.  In order to guarantee consistency and conservation,
$\mathcal{H}(\psi_{ij}, \psi_{ji}, {\bf n}_{ij})$ is required to satisfy

\begin{equation}
\mathcal{H}(\psi_{ij}, \psi_{ij}, {\bf n}_{ij}) = ({\bf v}_{ij}\cdot{\bf n}_{ij})\psi_{ij}
\end{equation}
and
\begin{equation}
\mathcal{H}(\psi_{ij}, \psi_{ji}, {\bf n}_{ij}) = -\mathcal{H}(\psi_{ji}, \psi_{ij}, {\bf n}_{ij}).
\end{equation}

Similarly, the flux function on the domain boundary, $(\psi_{ib}{\bf v}_{ib})\cdot{\bf n}_{ib}$,
should be determined by $\mathcal{H}(\psi_{ib}, \psi_b, {\bf n}_{ib})$, i.e.,
\begin{equation}
\mathcal{H}(\psi_{ib}, \psi_b, {\bf n}_{ib}) \approx (\psi_{ib}{\bf v}_{ib})\cdot{\bf n}_{ib},
\end{equation}
with the use of appropriate boundary conditions satisfying the characteristic theory.

Finally, the boundary integration in [eq:fvm_divergence_theorem_bc] is approximated using
one point quadrature at the midpoint of the face, and the semi-discrete form of the equations may be
written as
\begin{equation}
\label{eq:fvm_semi_discrete}
  V_i\frac{{\rm d}{\psi_i}}{{\rm d}t}
+ \sum_{j \in N_i}\mathcal{H}(\psi_{ij}, \psi_{ji}, {\bf n}_{ij})S_{ij}
+ \sum_{\Gamma_{ib} \in \partial\Omega}\mathcal{H}_b(\psi_{ib}, \psi_b, {\bf n}_{ib})S_{ib}
= 0,
\end{equation}
where $S_{ij}$ is the length of cell edge in 2D, and area of cell face in 3D.

By assembling all the elemental contributions, a system of ordinary differential equations
governing the evolution of the discrete solution in time can be written as
\begin{equation}
{\bf{\it M}}\frac{{\rm d}\psi}{{\rm d}t} = -{\bf R}(\psi),
\end{equation}
where $\bf{\it M}$ denotes the mass matrix, $\bf W$ is the global vector of the degrees of freedom,
and ${\bf R}(\psi)$ is the residual vector.  $\bf{\it M}$ has a block diagonal structure that couples
the degrees of freedom of the unknown vector associated to $\psi_i$ only within $\Omega_i$.  As a
result, the inverse of $\bf{\it M}$ can be easily computed in advance considering one cell at a time.

## Numerical Flux Scheme

In the example of advection equation, the flux function is approximated using the upwind scheme, i.e.,
\begin{equation}
\mathcal{H}(\psi_{ij}, \psi_{ji}, {\bf n}_{ij}) = a^+\psi_i + a^-\psi_j,
\end{equation}
where
\begin{equation}
a^+ = \frac{1}{2}(v^{\perp}_{ij} + |v^{\perp}_{ij}|)
\end{equation}
and
\begin{equation}
a^- = \frac{1}{2}(v^{\perp}_{ji} - |v^{\perp}_{ji}|),
\end{equation}
where
\begin{equation}
v^{\perp}_{ij} = {\bf v}_{ij}\cdot{\bf n}_{ij}
\end{equation}
and
\begin{equation}
v^{\perp}_{ji} = {\bf v}_{ji}\cdot{\bf n}_{ij}.
\end{equation}

## TVD Slope Limiters

First-order FVMs are in general stable on arbitrary grids.  However, second-order FVMs based on
piecewise linear reconstruction suffer from non-physical oscillations in the vicinity of strong
discontinuities for convection-dominant flows.  One common approach to address this issue is an
appropriate slope limiter.  Slope limiters are widely used in FVMs to modify the piecewise linearly
reconstructed gradients of solution variables, and thus to satisfy the total-variational diminishing
(TVD) condition.

In MOOSE, implementation of the TVD slope limiters is possible on 1D unstructured grids, because it
is trivial to know the indices of the "left" and "right" neighboring elements of the $i$-th element,
i.e., $i-1$ and $i+1$, during a loop over the elements.  The three classical slope limiters
implemented in the example of advection equation are described below.

### Minmod Slope Limiter

One choice of slope that gives second-order accuracy for smooth solutions while still satisfying the TVD property is the *minmod slope*
\begin{equation}
\phi_i = {\rm minmod}
\left(
\frac{\psi_i-\psi_{i-1}}{\Delta x},
\frac{\psi_{i+1}-\psi_i}{\Delta x}
\right)
\end{equation}
where the minmod function of two arguments is defined by
\begin{equation}
{\rm minmod}(a, b) =
\begin{cases}
a & \text{ if } |a| < |b| \text{ and } ab>0,\\
b & \text{ if } |b| < |a| \text{ and } ab>0,\\
0 & \text{ if } ab \le 0.
\end{cases}
\end{equation}
If $a$ and $b$ have the same sign, then this selects the one that is smaller in modulus, else it
returns zero.

Rather than defining the slope on the $i$-th cell by always using the downwind difference (which
would give the Lax-Wendroff method), or by always using the upwind difference (which would give the
Beam-Warming method), the minmod method compares the two slopes and chooses the one that is smaller
in magnitude. If the two slopes have different sign, then the value $\psi_i$ must be a local maximum
or minimum, and it is easy to check in this case that we must set $\phi_i = 0$ in order to satisfy
the TVD condition. The minmod method does a fairly good job of maintaining good accuracy in the
smooth hump and also sharp discontinuities in the square wave, with no oscillations. Sharper
resolution of discontinuities can be achieved with other limiters that do not reduce the slope as
severely as minmod near a discontinuity.

### Superbee Slope Limiter

One choice of limiter that gives the sharper reconstruction, while still giving second order accuracy
for smooth solutions, is the so-called *superbee* limiter introduced by [!cite](roe1985some):
\begin{equation}
\label{eq:superbee-limiter}
\phi_i={\rm maxmod}(\phi^{(1)}_i,\phi^{(2)}_i),
\end{equation}
where
\begin{equation}
\phi^{(1)}_i = {\rm minmod}
\left(
\frac{\psi_{i+1}-\psi_i}{\Delta x},
2\frac{\psi_i-\psi_{i-1}}{\Delta x}
\right)
\end{equation}
and
\begin{equation}
\phi^{(2)}_i = {\rm minmod}
\left(
2\frac{Q_{i+1}-Q_i}{\Delta x},
\frac{Q_i-Q_{i-1}}{\Delta x}
\right).
\begin{equation}
Each one-sided slope is compared with twice the opposite one-sided slope.  Then the maxmod function
in [eq:superbee-limiter] selects the argument with larger modulus.  In regions where the
solution is smooth this will tend to return the larger of the two one-sided slopes, but will still be
giving an approximation, and hence we expect second-order accuracy.  The superbee limiter is also TVD
in general.

With the superbee method, the discontinuity stays considerably sharper than with the minmod
method. On the other hand, there is a tendency of the smooth hump to become steeper and squared
off. This is sometimes a problem with superbee --- by choosing the larger of the neighboring slopes
it tends to steepen smooth transitions near inflection points.

### MC Slope Limiter

Another popular choice is the *monotonized central-difference limiter* (MC limiter), which was proposed by [!cite](van1977towards):
\begin{equation}
\label{eq:mc-limiter}
\phi_i = {\rm minmod}
\left(
\left(\frac{Q_{i+1}-Q_{i-1}}{2\Delta x}\right),
2\left(\frac{Q_i-Q_{i-1}}{\Delta x}\right),
2\left(\frac{Q_{i+1}-Q_i}{\Delta x}\right)
\right).
\end{equation}
This compares the central difference of Fromm method with twice the one-sided slope to either
side. In smooth regions this reduces to the centered slope of Fromm method and hence does not tend to
artificially steepen smooth slopes to the extent that superbee does. The MC limiter appears to be a
good default choice for a wide class of problems.

## Example Problem id=example

### Problem Description

In a test case validating the rDG implementation of the advection equation, the three slope limiters
introduced above are used for simulating a right-going square-shaped wave in 1D.  The initial
condition (I.C.) at $t=0$ contains contact discontinuities at $x=0.1$ and $x=0.6$.  For simplicity,
the wave speed is set to $1$.

!media anim_1d_aefv_square_wave.gif
       id=square_wave
       style=width:50%;margin-left:20px;float:right;
       caption=Time evolution of rDG solution for simulating a right-going square-shaped wave in 1D.g

### Numerical Results

To demonstrate the oscillation-free solution quality during wave propagation, an animation is
presented below.  In addition, the numerical results at $t=0.2$ are presented in [square_wave].

### Input File

The content of some input file blocks is described in detail for clarity.

#### GlobalParams

1. It is mandatory to declare `order = CONSTANT` and `family = MONOMIAL`, which specifies the
   piecewise cell-average solution variable for the cell-centered finite volume method.
2. It is convenient to provide some parameters for rDG-related objects used in an input file, such as
   `slope_reconstruction = rslope` and `slope_limiting = lslope` for *slope reconstruction* and
   *slope limiting*.
3. If an explicit time integration method is used, it is convenient to declare `implicit = false`
   here, so that Jacobian matrices will not be computed.


!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=GlobalParams

#### Mesh

!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=Mesh

Notes:

1. A one-dimensional computational domain ranging between $x=0$ and $x=1$ is defined, with 100
   elements equally distributed in the domain.

#### Functions

!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=Functions

Notes:

1. In this case, a piecewise constant function is used to specify the initial condition of the
   square-shaped wave profile.

#### UserObjects

!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=UserObjects

Notes:

1. The prefix `AEFV` represents "Advection Equation Finite Volume" --- an abbreviation to uniquely
   name these classes.
2. `AEFVSlopeReconstructionOneD` does not do any work, but has to be in place for code
   consistency. In 1D the slope reconstruction and limiting can be accomplished in one user object.
3. `AEFVSlopeLimitingOneD` calculates the limited slope for each element in 1D.
4. `AEFVUpwindInternalSideFlux` calculates the internal side flux using a simple upwind scheme.
5. `AEFVFreeOutflowBoundaryFlux` calculates the boundary side flux using a free outflow BC.

#### Variables

!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=Variables

Notes:

1. Declare the nonlinear variable as `u`. The type and family of the variable have been declared in
  `GlobalParams` for convenience.

#### Kernels

!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=Kernels

Notes:

1. Always set `implicit = true` for time derivative kernels when using explicit time integration.
2. In FVMs, there is no volumetric integration for flux terms.
3. Other possible kernels in this block include source terms. In this example, we do not have any.

#### DGKernels

!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=DGKernels

Notes:

1. Internal side flux terms should be declared in this block.

#### BCs

!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=BCs

Notes:

1. Boundary side flux terms should be declared in this block.

#### Materials

!listing modules/rdg/test/tests/advection_1d/1d_aefv_square_wave.i block=Materials

Notes:

1. This block does not calculate actual material properties. It is used to trigger the calculation of slopes in every element, and then interpolate variable values at side centers.

## Reference

!bibtex bibliography
