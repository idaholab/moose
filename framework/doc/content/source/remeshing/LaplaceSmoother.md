# LaplaceSmoother

!syntax description /Remeshing/Smoothers/LaplaceSmoother

## Description

Each spatial component $i$ of the pseudo-displacement $\mathbf{d}$ accumulated since the last mesh
replacement solves

\begin{equation}
\int_{\Omega_0} \nabla d_i \cdot \nabla \psi \, \mathrm{d}\Omega = 0 ,
\qquad
d_i = d_i^{\mathrm{prev}} + \Delta t \, v_i \ \text{ on } \Gamma_{\mathrm{mov}} ,
\qquad
d_i = 0 \ \text{ on } \Gamma_{\mathrm{fix}} ,
\end{equation}

where $\psi$ is a first order Lagrange test function, $\Omega_0$ is the configuration snapshotted at
the last replacement, $\Gamma_{\mathrm{mov}}$ are the boundaries named in
[!param](/Remeshing/Smoothers/LaplaceSmoother/moving_boundaries), $\Gamma_{\mathrm{fix}}$ the
boundaries named in [!param](/Remeshing/Smoothers/LaplaceSmoother/fixed_boundaries), $v_i$ the
interface velocity component, $\Delta t$ the time step size about to be solved with, and
$d_i^{\mathrm{prev}}$ the value the previous step left on the moving boundary. What is solved for is
the total motion since the snapshot, not an
increment: the moving boundary values accumulate, while the interior is a fresh harmonic
interpolation of them every step. The maximum principle then keeps every interior node inside the
range the boundaries prescribe.

The operator depends only on $\Omega_0$, so it is assembled once per mesh topology and reassembled
after each mesh replacement. The linear solve is a standalone one and is not part of the nonlinear
problem; a solve that fails to converge ends the run rather than leaving the mesh at the initial
guess.

The pseudo-displacement is discretized with first order Lagrange shape functions, which would leave
the mid-side nodes of a second order mesh unconstrained, so the mesh must be first order.

### Interface Velocity id=velocity

Supply exactly one of [!param](/Remeshing/Smoothers/LaplaceSmoother/velocity_functions), evaluated
at the end of the step at each moving boundary node, or
[!param](/Remeshing/Smoothers/LaplaceSmoother/velocity_variables), read at those nodes from nodal
Lagrange variables. Either takes one entry per mesh dimension, in order. Reading the velocity from
variables couples the mesh motion to a field the problem itself computes; supplying both, or
neither, is an error.

### Fixed and Sliding Boundaries id=fixed

Leaving [!param](/Remeshing/Smoothers/LaplaceSmoother/fixed_boundaries) unset pins every external
boundary of the mesh that is not a moving boundary, and rederives that set after each replacement,
since the surgery can change which sides have no neighbor. A node on both a moving and a fixed
boundary follows the moving boundary.

A fixed wall that shares a corner node with a moving boundary lets its nodes slide along itself:
only the components its reference normal points along are pinned, so a flat axis-perpendicular wall
pins its normal component alone. Without that, the corner travelling along the wall overtakes its
stationary neighbors and folds the boundary onto itself. An oblique wall has a normal with more than
one non-zero component and is therefore fully pinned, as is every fixed wall that no moving boundary
touches.

## Example Input File Syntax

A circular interface driven by prescribed velocity functions, with the outer walls pinned by
default:

!listing test/tests/remeshing/ale_remesh_reset.i block=Remeshing

!syntax parameters /Remeshing/Smoothers/LaplaceSmoother

!syntax inputs /Remeshing/Smoothers/LaplaceSmoother

!syntax children /Remeshing/Smoothers/LaplaceSmoother
