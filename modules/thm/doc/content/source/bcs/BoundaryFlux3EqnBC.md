!syntax description /BCs/BoundaryFlux3EqnBC

This implements a general boundary condition for the 1-D, 1-phase, variable-area Euler
equations using a boundary flux user object, which directly supplies the
boundary flux $\mathbf{F}_b$. Since a Riemann solver is not necessarily used,
one must be careful to obey characteristic theory when implementing the boundary
flux user objects.

!syntax parameters /BCs/BoundaryFlux3EqnBC

!syntax inputs /BCs/BoundaryFlux3EqnBC

!syntax children /BCs/BoundaryFlux3EqnBC
