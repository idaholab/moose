# ConservativeAdvection

## Description

The `ConservativeAdvection` kernel implements an advection term given for the domain ($\Omega$) defined as

\begin{equation}
\underbrace{\nabla \cdot \vec{v} u}_{\textrm{ConservativeAdvection}} + \sum_{i=1}^n \beta_i = 0 \in \Omega,
\end{equation}
where $v$ is the advecting velocity and the second term on the left hand side
represents the strong forms of other kernels. `ConservativeAdvection` does not assume
that the velocity is divergence free and instead applies $\nabla$ to the test
function $\psi_i$ in the weak variational form after integrating by parts,
which results in the following (without numerical stabilization)

\begin{equation}
R_i(u_h) = \underbrace{-(\nabla \psi_i, \vec{v} u)}_{\textrm{ConservativeAdvection}} + \langle\psi_i, \vec{v} u
\cdot \vec{n}\rangle \quad \forall \psi_i,
\end{equation}
where $\psi_i$ are the test functions and $u_h \in \mathcal{S}^h$ is the finite
element solution of the weak formulation. The first term is the volumetric term and the second term
is a surface term describing the advective flux out of the
volume. `ConservativeAdvection` corresponds to the former volumetric term, while the surface term is implemented as a BC in MOOSE (see discussion below regarding `OutflowBC` and `InflowBC`).

Without numerical stabilization the corresponding Jacobian is given by

\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} = -(\nabla \psi_i, \vec{v} \phi_j).
\end{equation}

## Full upwinding

Advective flow is notoriously prone to physically-incorrect overshoots
and undershoots, so in many simulations some numerical stabilization
is used to reduce or eliminate this spurious behaviour.
Full-upwinding [!cite](dalen1979,adhikary2011) is an example of
numerical stabilization and this essentially adds numerical diffusion
to completely eliminate overshoots and undershoots.  Full-upwinding is
available in `ConservativeAdvection` by setting the `upwinding_type`
appropriately.

!alert note
Full upwinding only works for continuous FEM

In DE systems describing more than just advection (e.g., in
diffusion-advection problems) full upwinding may be used on the
advection alone.  In practice, it is reasonably common that these more
complicated cases benefit from numerical stabilization on their
other terms too, in which case full upwinding could also be used, but
this is not mandatory and is not implemented in the
`ConservativeAdvection` Kernel.

For each element in the mesh, full upwinding is implemented in the
following way.  For each $i$, the quantity
\begin{equation}
\tilde{R}_{i} = -(\nabla\psi_i, \vec{v}) \ ,
\end{equation}
is evaluated.  If $\tilde{R}_{i}>0$ is positive then mass (or heat, or whatever $u$
represents) is flowing *out* of node $i$, and this is called an
"upwind" node.  If $\tilde{R}_{i}\geq 0$, the residual for node $i$
is set to
\begin{equation}
R_{i} = u_{i}\tilde{R}_{i} \ \ \texttt{ for } \ \ \tilde{R}_{i}\geq 0.
\end{equation}
Here $u_{i}$ is the value of $u$ at the node $i$.  Define the total
mass flowing from the upwind nodes:
\begin{equation}
M_{\mathrm{out}} = \sum_{i\ \mathrm{ with }\ \tilde{R}_{i}\geq 0}
u_{i}\tilde{R}_{i} \ .
\end{equation}
Similarly, define
\begin{equation}
\tilde{M}_{\mathrm{in}} = - \sum_{i \ \mathrm{ with }\  \tilde{R}_{i}< 0}
\tilde{R}_{i} \ .
\end{equation}
Mass is conserved if the residual for the downwind nodes is defined to
be
\begin{equation}
R_{i} = \tilde{R}_{i} M_{\mathrm{out}} / M_{\mathrm{in}}  \ \ \texttt{ for } \ \ \tilde{R}_{i}< 0.
\end{equation}

Full upwinding adds more numerical diffusion than most other numerical
stabilization techniques such as SUPG and TVD.  Another problem is
that steady-state can be hard to achieve in nonlinear problems where
the velocity is not fixed and changes every nonlinear iteration (this
does not occur for `ConservativeAdvection`).  On the other hand, full
upwinding is computationally cheap.



## Example Syntax

`ConservativeAdvection` can be used in a variety of problems, including
advection-diffusion-reaction. The syntax for `ConservativeAdvection` is
demonstrated in this `Kernels` block from a pure advection test case:

!listing test/tests/kernels/conservative_advection/no_upwinding_1D.i block=Kernels

The velocity is supplied as a three component vector with order $v_x$,
$v_y$, and  $v_z$.

## Boundary conditions for pure advection

To form the correct equations, the boundary term $\langle\psi_i, \vec{v} u
\cdot \vec{n}\rangle$ needs to be included in the `BCs` block of a
MOOSE input file ($\vec{n}$ is the outward normal to the boundary).  An
`OutflowBC` may be used, for instance

!listing test/tests/kernels/conservative_advection/none_in_all_out.i block=BCs

Physically this subtracts $\langle\psi_i, \vec{v} u \cdot
\vec{n}\rangle$ "fluid mass" (or whatever $u$ represents) from the boundary.

For $\vec{v} \cdot \vec{n} > 0$ adding the `OutflowBC` allows "fluid" to flow
freely through the boundary. The advective velocity is "blowing fluid" into this boundary, and the `OutflowBC` is removing it at the correct rate, because the flux through any area is $\langle\psi_i, \vec{v} u \cdot
\vec{n}\rangle$.

On the other hand, including an `OutflowBC` for $\vec{v} \cdot \vec{n}
< 0$ isn't usually desirable.  Adding the
`OutflowBC` in this case fixes $u$ at the boundary to its initial
condition. This is because the `ConservativeAdvection` Kernel is taking
fluid from the boundary to the interior of the model, but at the
same time the `OutflowBC` is removing $\langle\psi_i, \vec{v} u \cdot
\vec{n}\rangle$: note that $\vec{v} \cdot \vec{n} < 0$, so the
`OutflowBC` is actually adding fluid at the same rate the
`ConservativeAdvection` Kernel is removing it. The physical interpretation
is that something external to the model is adding fluid at exactly the
rate specified by the initial conditions at that boundary.

Instead, for $\vec{v} \cdot \vec{n} < 0$ users typically want to specify a particular value for an
injected flux.  This is achieved by using an `InflowBC`.  This adds $\langle\psi_i, \vec{v} u_{B} \cdot
\vec{n}\rangle$, where $u_B$ (kg.m$^{-2}$.s$^{-1}$) is the injection
rate.  For instance

!listing test/tests/kernels/conservative_advection/full_upwinding_1D.i block=BCs

To make impermeable boundaries, either for $\vec{v} \cdot \vec{n} < 0$
or $\vec{v} \cdot \vec{n} > 0$, simply use
no BC at that boundary. Then for $\vec{v} \cdot \vec{n} > 0$ there is no BC to remove fluid
from that boundary so the fluid "piles up" there. For $\vec{v} \cdot \vec{n} < 0$ the
omission of a BC can be thought of setting $u_B=0$ in an
`InflowBC`.

## Comparison of no upwinding and full upwinding

The tests

!listing test/tests/kernels/conservative_advection/no_upwinding_1D.i

and

!listing test/tests/kernels/conservative_advection/full_upwinding_1D.i

describe the same physical situation: advection from left to right
along a line.  A source at the left boundary introduces $u$ into the
domain at a fixed rate (using an `InflowBC`).  The right boundary is
impermeable (no BC), so when $u$ arrives
there it starts building up at the boundary.  It is clear from the
figures below that no upwinding leads to unphysical overshoots and
undershoots, while full upwinding results in none of that oscillatory
behaviour but instead produces more numerical diffusion.

!media media/framework/kernels/conservative_advection_1d_1.png style=width:30%;margin-left:3% caption=$u$ after 0.1 seconds of advection

!media media/framework/kernels/conservative_advection_1d_5.png style=width:30%;margin-left:3% caption=$u$ after 0.5 seconds of advection

!syntax parameters /Kernels/ConservativeAdvection

!syntax inputs /Kernels/ConservativeAdvection

!syntax children /Kernels/ConservativeAdvection

!bibtex bibliography
