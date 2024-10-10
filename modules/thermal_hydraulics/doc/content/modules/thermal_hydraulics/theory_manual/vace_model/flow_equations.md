## Flow Equations

The governing flow equations for the variable-area formulation of the compressible
Euler equations are as follows:

!equation
\pd{\rho A}{t} + \pd{\rho u A}{x} = 0 \,,

!equation
\pd{\rho u A}{t} + \pd{(\rho u^2 + p) A}{x} = p \pd{A}{x} - F A + \rho g_x A \,,

!equation
\pd{\rho E A}{t} + \pd{u (\rho E + p) A}{x} = \rho u g_x A + q''' A \,,

where

- $t$ is time,
- $x$ is the spatial position along the chosen axis,
- $A$ is the cross-sectional area,
- $\rho$ is the density,
- $u$ is the axial velocity,
- $E$ is the specific total energy,
- $p$ is the pressure,
- $F$ is the viscous drag force density,
- $g_x$ is the component of acceleration due to gravity in the axial direction, and
- $q'''$ is the heat source rate density.

The viscous drag force density is computed as

!equation
F = \frac{f \rho u |u| A}{2 D_h} \,,

where $f$ is the Darcy friction factor and $D_h$ is the hydraulic diameter.
