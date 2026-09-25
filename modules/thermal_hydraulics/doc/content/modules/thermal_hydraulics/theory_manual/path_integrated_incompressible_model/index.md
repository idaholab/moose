# Path-Integrated Incompressible Flow Model

The Path-Integrated Incompressible Flow Model is used to model one-dimensional, single-phase flow using the incompressible Navier-Stokes equation in flow paths of variable geometry. The incompressible formulation used here is termed "globally compressible, locally incompressible", meaning the effects of natural circulation are accounted for in the gravitational acceleration term through variable density resulting from thermal expansion, but for all other terms a single, average density is used. Property changes resulting from pressure changes are not accounted for.

## Flow Equations

The governing flow equations for incompressible flow within a 1D flow path are as follows:

!equation id=mass_conservation
\pd{\dot{m}}{x} = 0 \,

!equation id=momentum_conservation
\frac{1}{A} \pd{\dot{m}}{t} = -\pd{P}{x} - \frac{f}{D_h} \frac{\dot{m}^2}{2\rho A^2} - \sum_{j} K_j \delta(z-z_j) \frac{\dot{m}^2}{2 \rho A^2} - \rho g \sin{\alpha} + \Delta P_p \,

!equation id=energy_conservation
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

!equation id=discretized_momentum
\sum_{i=1}^{N} \frac{L_i}{A_i} \pd{\dot{m}}{t} = - \Delta P_c - \sum_{i=1}^{N} \frac{f_i L_i}{D_{h,i}} \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i^2} - \sum_{i=1}^{N} K_i \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i^2} - \sum_{i=1}^{N} \rho_i g L_i \sin{\alpha_i} + \Delta P_p \,

where

- $L_i$ is the length of segment $i$, and
- $\Delta P_c$ is the characteristic pressure drop from inlet to outlet.

A core assumption of this model is that the flow path consists of N segments, that the segments may be geometrically unique between one another, but the geometry within a given segment is fairly consistent. For example, a segment may have elbows, support structures, etc. within it but the overall cross-section shouldn't change significantly over the length of the segment. Other properties are permitted to be unique to each segment as well, including viscosity, surface roughness, flow angle with respect to the horizontal, and length.

Input parameters are all given as functors, which enables flexibility in controllability, state changes in time, etc. However, caution should be exercised in changing parameters in time, as the parameters are assumed to be constant in time in the mathematical derivation. Hence, allowing the parameters to vary in time is intended only for long-running transients where the change over time is small, or for cases where the immediate transient effects of changing a parameter in time are unimportant to the phenomena of interest. This isn't the model to use for studying rapid transient effects.

To preserve proper friction force direction regardless of flow direction, $\dot{m}^2$ is replaced with $\dot{m}|\dot{m}|$ in the friction terms. Subscript $c$ in $\rho_c$ represents the characteristic (or average) density of the fluid along the flow path. The gravity term uses local density based on temperature to account for natural convection. This partitioning of the "global" density for most terms, but "local" density for the gravity term only results in a "globally compressible, locally incompressible" formulation that accounts for natural circulation but is otherwise incompressible in formulation.

Given the effect of local temperature on the momentum equation, the energy equation is discretized on a per-segment basis as follows:

!equation id=discretized_energy
A \rho c_p \pd{T}{t} + \frac{\dot{m}}{2 L} \left(1 - \frac{|\dot{m}|}{\dot{m}}\right) c_p T_d - \frac{\dot{m}}{2 L} \left(1 + \frac{|\dot{m}|}{\dot{m}}\right) c_p T_u + \frac{|\dot{m}| c_p}{L} T = \frac{h P_w}{2} \left( 2 T_w - T - \frac{1}{2} \left( 1 - \frac{|\dot{m}|}{\dot{m}} \right) T_d - \frac{1}{2} \left( 1 + \frac{|\dot{m}|}{\dot{m}} \right) T_u \right) \,

where

- $T_d$ is the downstream segment's temperature,
- $T_u$ is the upstream segment's temperature,
- $h$ is the heat transfer coefficient, and
- $T_w$ is the wall temperature.

The implication of this discretization is that the temperature of the segment is defined at the opposite end of the segment from the flow entrance. Hence, proper conservation of energy necessitates the modified wall heat transfer term following the form of Newton's law of cooling. Furthermore, a modified advection term is used to ensure the proper directionality of the advection regardless of the flow direction.

Most closure quantities in the governing equations are user-supplied quantities, including: flow area, reference pressure, forms loss coefficients, gravitational acceleration, segment angles, pump pressure gains, and wetted perimeters.

A few notable closure quantities are not user-specified inputs, and are worth some discussion.

The thermophysical properties ($\rho$, $\mu$, $k$, & $c_p$) are computed from the coupled temperature values and the reference pressure using a supplied [SinglePhaseFluidProperties.md] object. Most terms in the momentum equation utilize the average temperature from inlet to outlet of the flow path to compute the fluid properties, except for the gravitational term, which uses local temperature to model natural circulation effects. Since the temperature equation is solved on a segment-local basis, local temperature is used to compute the thermophysical properties.

The hydraulic diameter of each segment is computed using the closure relation:

!equation id=hydraulic_diameter
D_{h,i} = \frac{4 A_i}{P_{w,i}} \,

The friction factor of each segment is computed differently for laminar, turbulent, and transition flow. For $Re < 2300$ the flow is deemed laminar and the following analytical relation is used:

!equation id=laminar_friction
f = \frac{64}{Re} \,

For $Re > 4000$ the flow is deemed turbulent and the Swamee-Jain approximation of the Colebrook-White equation is used:

!equation id=swamee_jain_friction
f = \frac{0.25}{\left( \log_{10} \left( \frac{\epsilon}{3.7 D_{h,i}} + \frac{5.74}{Re^{0.9}} \right) \right)^2} \,

For $2300 < Re < 4000$ the flow is deemed be transitioning and a conservative interpolation between turbulent and laminar predictions is used:

!equation id=transition_friction
f = \max(\frac{f_{turb} - f_{lam}}{1700} f_{turb} + f_{lam}, \max(f_{turb}, f_{lam})) \,

The heat transfer coefficient is computed using the Dittus-Boelter correlation:

!equation id=dittus_boelter
h = 0.023 Re^{0.8} Pr^{0.4} \,

Future iterations of these kernels will see expanded options for closure correlations. Stay tuned!

## Junctions

The above equations are valid for 1D flow paths only; for a flow junction, the mass conservation equation is as follows:

!equation id=junction_mass_conservation
\dot{m}_{in} = \dot{m}_{out} \,

Typically, it is sufficient to assume that negligible momentum and energy transfer occurs at a junction.
