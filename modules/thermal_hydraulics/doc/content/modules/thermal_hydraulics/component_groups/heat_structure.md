# Heat Structures

A "heat structure" is here defined to be a multi-dimensional `Component` on which the
heat conduction equation is solved.

The following components are heat structures:

- [HeatStructureFromFile3D.md]: 3D heat structure using mesh provided by a file.
- [HeatStructureCylindrical.md]: 2D cylindrical heat structure that generates its own mesh.
- [HeatStructurePlate.md]: 2D rectangular plate heat structure that generates its own mesh.

## Usage

The initial temperature is given by the function parameter `initial_T`.

### Variables and Material Properties

This component creates the following variables:

| Variable | Symbol | Description |
| :- | :- | :- |
| `T_solid` | $T$ | Temperature \[K\] |

## Formulation

The heat conduction equation is the following:
\begin{equation}
  \rho c_p \pd{T}{t} - \nabla \cdot (k \nabla T) = 0 \eqc
\end{equation}
where

- $\rho$ is density,
- $c_p$ is specific heat capacity,
- $k$ is thermal conductivity, and
- $T$ is temperature.

Multiplying by a test function $\phi_i$ and integrating by parts over the domain
$\Omega$ gives
\begin{equation}
  \pr{\rho c_p \pd{T}{t}, \phi_i}_\Omega + \pr{k \nabla T, \nabla\phi_i}_\Omega
    - \left\langle k \nabla T, \phi_i\mathbf{n}\right\rangle_{\partial\Omega} = 0 \eqc
\end{equation}
where $\partial\Omega$ is the boundary of the domain $\Omega$.
