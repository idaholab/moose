# Path-Integrated Incompressible Flow Model

The Path-Integrated Incompressible Flow Model is used to model one-dimensional, single-phase flow using the incompressible Navier-Stokes equation in flow paths of variable geometry. The incompressible formulation used here is termed "globally compressible, locally incompressible", meaning the effects of natural circulation are accounted for in the gravitational acceleration term through variable density resulting from thermal expansion, but for all other terms a single, average density is used. Property changes resulting from pressure changes are not accounted for.

## Flow Equations

The governing flow equations for incompressible flow within a 1D flow path are as follows:

!equation id=mass_conservation
\pd{\dot{m}}{x} = 0 \,

!equation id=momentum_conservation
\frac{1}{A} \pd{\dot{m}}{t} = -\pd{P}{x} - \frac{f}{D_h} \frac{\dot{m}^2}{2\rho A^2} - \sum_{j} K_j \delta(z-z_j) \frac{\dot{m}^2}{2 \rho A^2} - \rho g \sin{\alpha} + \pd{P_p}{x} \,

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

A core assumption of this model is that the flow path consists of N segments, that the segments may have different cross sections, but within each segment, the cross section is assumed to be approximately constant. For example, a segment may have elbows, support structures, etc. within it but the overall cross section shouldn't change significantly over the length of the segment. Other properties are permitted to be unique to each segment as well, including viscosity, surface roughness, flow angle with respect to the horizontal, and length.

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

The thermophysical properties ($\rho$, $\mu$, $k$, and $c_p$) are computed from the coupled temperature values and the reference pressure using a supplied [SinglePhaseFluidProperties.md] object. Most terms in the momentum equation utilize the average temperature from inlet to outlet of the flow path to compute the fluid properties, except for the gravitational term, which uses local temperature to model natural circulation effects. Since the temperature equation is solved on a segment-local basis, local temperature is used to compute the thermophysical properties.

The hydraulic diameter of each segment is computed using the closure relation:

!equation id=hydraulic_diameter
D_{h,i} = \frac{4 A_i}{P_{w,i}} \,

The friction factor of each segment is computed differently for laminar, turbulent, and transition flow. For $Re < 2300$ the flow is deemed laminar and the following analytical relation is used:

!equation id=laminar_friction
f = \frac{64}{Re} \,

For $Re > 4000$ the flow is deemed turbulent and the Swamee-Jain approximation of the Colebrook-White equation is used [!citep](swamee_jain1976):

!equation id=swamee_jain_friction
f = \frac{0.25}{\left( \log_{10} \left( \frac{\epsilon}{3.7 D_{h,i}} + \frac{5.74}{Re^{0.9}} \right) \right)^2} \,

For $2300 < Re < 4000$ the flow is deemed be transitioning and a conservative interpolation between turbulent and laminar predictions is used:

!equation id=transition_friction
f = \max\left(\frac{f_{turb} - f_{lam}}{1700} f_{turb} + f_{lam}, \max(f_{turb}, f_{lam})\right) \,

The heat transfer coefficient is computed using the Dittus-Boelter correlation [!citep](dittus1930heat):

!equation id=dittus_boelter
h = 0.023 Re^{0.8} Pr^{0.4} \,

## Junctions

The above equations are valid for 1D flow paths only; for a flow junction, the mass conservation equation is as follows:

!equation id=junction_mass_conservation
\dot{m}_{in} = \dot{m}_{out} \,

Typically, it is sufficient to assume that negligible momentum and energy transfer occurs at a junction.

## Conjugate Heat Transfer

The flow energy equation above is designed to couple to a variable segment wall temperature. For convenience and modularity, additional conjugate heat transfer kernels are added to close the heat transfer through the solid bounding wall. The area-averaged governing equation for which is:

! equation id=pipe_CHT_conservation
A_w \rho_w c_{p,w} \pd{T_w}{t} = \nabla \cdot \left (k_w \nabla T_w \right ) + q_{i,w}^{''} P_{i,w} - q_{o,w}^{''} P_{o,w}

where

- $A_w$ is the cross-sectional area of the wall,
- $\rho_w$ is the density of the wall,
- $c_{p,w}$ is the specific isobaric heat capacity of the wall,
- $T_w$ is the temperature of the wall,
- $k_w$ is the thermal conductivity of the wall,
- $q_{i,w}^{''}$ is the heat flux entering the inner surface of the wall,
- $q_{o,w}^{''}$ is the heat flux exiting the outer surface of the wall,
- $P_{i,w}$ is the perimeter of the inner surface of the cross-section, and
- $P_{o,w}$ is the perimeter of the outer surface of the cross-section.

This equation is discretized on a per-segment basis to match what is done in [!eqref](discretized_energy). Furthermore, the wall is discretized into two nodes through the thickness to capture the inner wall temperature and the outer wall temperature, as shown in [fig:pipe_CHT_discretization].

!media thermal_hydraulics/tikz_diagrams/pipe_cross_section.png
       id=fig:pipe_CHT_discretization
       caption=Conjugate heat transfer illustration for a simple pipe.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;text-align:center;

Thus, the discretized energy equation for the inner wall temperature is:

! equation id=discretized_inner_pipe_temperature
A_{i,w} \rho_w c_{p,w} \pd{T_{i,w}}{t} = \frac{k_w P_{m,w} \left( T_{o,w} - T_{i,w} \right)}{\delta} +
\frac{G_{i,u} \left( T_{i,u} - T_{i,w} \right) + G_{i,d} \left( T_{i,d} - T_{i,w} \right)}{L} +
h \frac{P_{i,w}}{2} \left( T_f + T_{in} - 2T_{i,w} \right)

where

- $A_{i,w}$ is the cross-sectional area of the inner layer of the wall,
- $T_{i,w}$ is the temperature of the inner surface of the wall,
- $T_{o,w}$ is the temperature of the outer surface of the wall,
- $P_{m,w}$ is the perimeter of the mid-thickness dividing line that represents the boundary between the inner layer and the outer layer of the wall,
- $L$ is the segment length,
- $h$ is the heat transfer coefficient of the fluid,
- $T_f$ is the fluid temperature of the segment (defined at the outlet of the segment, as mentioned before), and
- $T_{in}$ is the fluid temperature at the inlet of the segment.

$G_{i,u}$ and $G_{i,d}$ are the axial face conductances to the upstream and downstream inner wall nodes. Each is the harmonic mean of two half-length conduction resistances in series across the face, accounting for the upstream/downstream node potentially having a different cross-section than this node:

!equation id=upstream_axial_conductance
G_{i,u} = \left( \frac{\Delta x_u}{2 k_w A_{i,w}} + \frac{\Delta x_u}{2 k_u A_{i,u}} \right)^{-1}

!equation id=downstream_axial_conductance
G_{i,d} = \left( \frac{\Delta x_d}{2 k_w A_{i,w}} + \frac{\Delta x_d}{2 k_d A_{i,d}} \right)^{-1}

where

- $\Delta x_u$ and $\Delta x_d$ are the axial spacings to the upstream and downstream nodes,
- $k_u$ and $k_d$ are the wall conductivity evaluated at the upstream and downstream node's own temperature, and
- $A_{i,u}$ and $A_{i,d}$ are the layer area evaluated at the upstream and downstream node's own cross-section.

Similarly, the discretized energy equation for the outer wall temperature is:

!equation id=discretized_outer_pipe_temperature
A_{o,w} \rho_w c_{p,w} \pd{T_{o,w}}{t} = \frac{k_w P_{m,w} \left( T_{i,w} - T_{o,w} \right)}{\delta} +
\frac{G_{o,u} \left( T_{o,u} - T_{o,w} \right) + G_{o,d} \left( T_{o,d} - T_{o,w} \right)}{L} + q_o^{'}

where

- $q_o^{'}$ is the per-length heat transfer term through the outer boundary.

Since the outer boundary of the wall may be subject to a number of boundary conditions, a number of kernels are provided for flexibility in modeling the per-length heat transfer term. The base kernels include:
- Ambient convection [PipeOuterWallAmbientTemperatureScalarKernel.md]
- Coupled secondary-side convection [PipeOuterWallCoupledConvectiveTemperatureScalarKernel.md]
- Applied heat flux [PipeOuterWallHeatFluxScalarKernel.md]

An optional standalone term for radiative heat transfer [PipeOuterWallRadiationScalarKernel.md] may also be added to any of the base outer wall temperature kernels (but it really only makes sense for the ambient and coupled convection kernels).
