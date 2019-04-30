# AdvectionBC

!syntax description /BCs/AdvectionBC

This boundary condition accounts for advection of a scalar quantity $\phi$
by velocity $\vec{v}$.

It is usually applied on outflow boundary conditions and originates
from integration by parts of $\nabla \cdot (\vec{v} \phi)$. Multiplying by
test function $\psi$ and using Green's identity gives:
\begin{equation}
  \int_{\Omega}  \psi \nabla \cdot (\vec{v} \phi) d \Omega = - \int_{\Omega}  \nabla \psi  \cdot (\vec{v} \phi) d \Omega
  +  \int_{\partial \Omega}  \psi  \phi \vec{n} \cdot \vec{v}  d S.
\end{equation}
The boundary $\partial \Omega$ can be divided into two parts, inflow and outflow satsifying:
\begin{equation}
\begin{aligned}
\partial \Omega^-:&\vec{n} \cdot \vec{v} < 0 \\
\partial \Omega^+:&\vec{n} \cdot \vec{v} > 0
\end{aligned}
\end{equation}
Boundary conditions are applied on the inflow boundaries, e.g. the value of
$\phi$ may be known and is set by a Dirichlet boundary conditions. However, term:
\begin{equation}
  \int_{\partial \Omega^+}  \psi  \phi \vec{n} \cdot \vec{v} d S.
\end{equation}
still needs to be applied in the PDE. This is covered by AdvectionBC.

!syntax parameters /BCs/AdvectionBC

!syntax inputs /BCs/AdvectionBC

!syntax children /BCs/AdvectionBC
