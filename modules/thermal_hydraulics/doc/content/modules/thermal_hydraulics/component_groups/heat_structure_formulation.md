The heat conduction equation is the following:
\begin{equation}
  \rho c_p \pd{T}{t} - \nabla \cdot (k \nabla T) = q''' \eqc
\end{equation}
where

- $\rho$ is density,
- $c_p$ is specific heat capacity,
- $k$ is thermal conductivity,
- $T$ is temperature, and
- $q'''$ is a volumetric heat source.

Multiplying by a test function $\phi_i$ and integrating by parts over the domain
$\Omega$ gives
\begin{equation}
  \pr{\rho c_p \pd{T}{t}, \phi_i}_\Omega + \pr{k \nabla T, \nabla\phi_i}_\Omega
    - \left\langle k \nabla T, \phi_i\mathbf{n}\right\rangle_{\partial\Omega}
    = \pr{q''', \phi_i}_\Omega \eqc
\end{equation}
where $\partial\Omega$ is the boundary of the domain $\Omega$.
