# ADArrayRobinBC

!syntax description /BCs/ADArrayRobinBC

The boundary condition $\nabla \vec{u} \cdot \vec{n} = \vec{h} - \vec{\alpha} \circ \vec{u}$
contributes

\begin{equation}
\int_{\partial \Omega} \left( \vec{\alpha} \circ \vec{u} - \vec{h} \right) \vec{\psi} \, ds
\end{equation}

to the weak form of a diffusion operator, where $\vec{\psi}$ is the test function and $\circ$ is
the component-wise product. With
$\vec{\alpha} = 0$ it is a Neumann boundary condition like [ArrayNeumannBC.md]. The Jacobian is
computed with automatic differentiation by the base class `ADArrayIntegratedBC`, which provides
AD residuals for integrated boundary conditions on array variables.

## Example Input Syntax

!listing test/tests/bcs/ad_array_integrated_bc/ad_array_robin.i block=BCs

!syntax parameters /BCs/ADArrayRobinBC

!syntax inputs /BCs/ADArrayRobinBC

!syntax children /BCs/ADArrayRobinBC
