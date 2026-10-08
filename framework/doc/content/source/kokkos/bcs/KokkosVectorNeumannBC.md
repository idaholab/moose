# KokkosVectorNeumannBC

!if! function=hasCapability('kokkos')

!syntax description /BCs/KokkosVectorNeumannBC

## Description

`KokkosVectorNeumannBC` imposes a constant flux on each component of a vector variable along a
boundary. It is the vector analogue of [KokkosNeumannBC](KokkosNeumannBC.md), and adds the integrated
boundary term

\begin{equation}
-(\vec{h}, \vec{\psi_i})_{\partial \Omega}
\end{equation}

to the residual, where $\vec{\psi_i}$ is the vector test function and $\vec{h}$ is the flux given by
[!param](/BCs/KokkosVectorNeumannBC/values). For a diffusion-like operator this prescribes
$\frac{\partial \vec{u}}{\partial n} = \vec{h}$ on the boundaries listed in
[!param](/BCs/KokkosVectorNeumannBC/boundary).

The variable named by [!param](/BCs/KokkosVectorNeumannBC/variable) must belong to a vector family
such as `LAGRANGE_VEC`, since the three entries of [!param](/BCs/KokkosVectorNeumannBC/values) are
applied to the components of a single vector variable rather than to separate variables. Components
beyond the mesh dimension are ignored.

[!param](/BCs/KokkosVectorNeumannBC/values) is controllable, so the flux can be varied during a
simulation through the [Controls](syntax/Controls/index.md) system.

## Example Input Syntax

In this example the vector variable `u` is held at zero on the `left` boundary and given a flux of 1
in $x$ and 2 in $y$ on the `right` boundary. With a vector diffusion operator on the unit square the
solution is then $u_x = x$ and $u_y = 2x$.

!listing test/tests/kokkos/bcs/vector_neumann/kokkos_vector_neumann.i block=BCs

!syntax parameters /BCs/KokkosVectorNeumannBC

!syntax inputs /BCs/KokkosVectorNeumannBC

!syntax children /BCs/KokkosVectorNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md
