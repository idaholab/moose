# Path-Integrated Incompressible Flow Model

The Path-Integrated Incompressible Flow Model is used to model one-dimensional, single-phase flow using the incompressible Navier-Stokes equation in flow paths of variable geometry.

## Flow Equations

The governing flow equations for incompressible flow within a 1D flow path are as follows:

!equation
\pd{\dot{m}}{x} = 0 \,

!equation
\frac{1}{A} \pd{\dot{m}}{t} = -\pd{P}{x} - \frac{f}{D_h} \frac{\dot{m}^2}{2\rho A^2} - \sum_{j} K_j \delta(z-z_j) \frac{\dot{m}^2}{2 \rho A^2} - \rho g \sin{\alpha} + \Delta P_p \,

!equation
A \rho c_p \pd{T}{t} + \dot{m} c_p \pd{T}{x} = q_w^{''} P_w \,

where

- $t$ is time,
- $x$ is the spatial position along the chosen axis,
- $\dot{m}$ is the mass flow rate,
- $A$ is the cross-sectional area,
- $P$ is the pressure,
- $f$ is the friction factor,
- $D_h$ is the hydraulic diameter,
- $\rho$ is the density,
- $K_j$ is the $j^{th}$ minor loss coefficient,
- $\delta$ is the dirac delta function,
- $g$ is the gravitational acceleration in the downward direction,
- $\alpha$ is the angle of the flow path with respect to horizontal,
- $\Delta P_p$ is the pump pressure increase in the flow path,
- $c_p$ is the specific isobaric heat capacity of the fluid,
- $T$ is the bulk fluid temperature,
- $q_w^{''}$ is the heat flux through the bounding surface(s), and
- $P_w$ is the wetted perimeter.

## Discretization

Integrating the above momentum equation along a 1D flow path consisting of $N$ segments results in the following equations, ignoring conservation of mass since it has been substituted in the conservation of momentum and energy equations already:

!equation
\sum_{i=1}^{N} \frac{L_i}{A_i} \pd{\dot{m}}{t} = - \Delta P_c - \sum_{i=1}^{N} \frac{f_i L_i}{D_{h,i}} \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i^2} - \sum_{i=1}^{N} K_i \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i^2} - \sum_{i=1}^{N} \rho_i g L_i \sin{\alpha_i} + \Delta P_p \,

where

- $L_i$ is the length of segment $i$, and
- $\Delta P_c$ is the characteristic pressure drop from inlet to outlet.

To preserve proper friction force direction regardless of flow direction, $\dot{m}^2$ is replaced with $\dot{m}|\dot{m}|$ in the friction terms. Subscript $c$ in $\rho_c$ represents the characteristic (or average) density of the fluid along the flow path. The gravity term uses local density based on temperature to account for natural convection.

Given the effect of local temperature on the momentum equation, the energy equation is discretized on a per-segment basis as follows:

!equation
A \rho c_p \pd{T}{t} + \frac{\dot{m}}{2 L} \left(1 - \frac{|\dot{m}|}{\dot{m}}\right) c_p T_d - \frac{\dot{m}}{2 L} \left(1 + \frac{|\dot{m}|}{\dot{m}}\right) c_p T_u + \frac{|\dot{m}| c_p}{L} T = \frac{h P_w}{2} \left( 2 T_w - T - \frac{1}{2} \left( 1 - \frac{|\dot{m}|}{\dot{m}} \right) T_d - \frac{1}{2} \left( 1 + \frac{|\dot{m}|}{\dot{m}} \right) T_u \right) \,

where

- $T_d$ is the downstream segment's temperature,
- $T_u$ is the upstream segment's temperature,
- $h$ is the heat transfer coefficient, and
- $T_w$ is the wall temperature.

The implication of this discretization is that the temperature of the segment is defined at the opposite end of the segment from the flow entrance. Hence, proper conservation of energy necessitates the modified wall heat transfer term following the form of Newton's law of cooling. Furthermore, a modified advection term is used to ensure the proper directionality of the advection regardless of the flow direction.

## Junctions

The above equations are valid for 1D flow paths only; for a flow junction, the mass conservation equation is as follows:

!equation
\dot{m}_{in} = \dot{m}_{out} \,

Typically, it is sufficient to assume that negligible momentum and energy transfer occurs at a junction.
