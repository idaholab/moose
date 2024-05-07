# DashpotBC

!syntax description /BCs/DashpotBC

The boundary condition represents a friction term proportional to the velocity.
The local contribution to the residual is computed as:

!equation
(\psi, K \vec{n} \cdot \vec{v})

where $K$ is the friction coefficient, $\vec{n}$ the local surface normal and
$\vec{v}$ is the velocity, computed from the displacements.

!alert note
This boundary condition hard-codes an implicit Euler time integration scheme in its
contribution to the Jacobian.

!syntax parameters /BCs/DashpotBC

!syntax inputs /BCs/DashpotBC

!syntax children /BCs/DashpotBC