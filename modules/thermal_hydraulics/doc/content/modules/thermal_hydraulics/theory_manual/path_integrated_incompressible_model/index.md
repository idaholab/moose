# Path-Integrated Incompressible Flow Model

The Path-Integrated Incompressible Flow Model is used to model one-dimensional, single-phase flow using the incompressible Navier-Stokes equation in flow paths of variable geometry. The incompressible formulation used here is termed "globally compressible, locally incompressible", meaning the effects of natural circulation are accounted for through temperature-dependent variable density resulting from thermal expansion. Property changes resulting from pressure changes are not accounted for.

## Flow Equations

The governing flow equations for incompressible flow within a 1D flow path are as follows:

!equation id=mass_conservation
\pd{\dot{m}}{x} = 0 \,

!equation id=momentum_conservation
\frac{1}{A} \pd{\dot{m}}{t} = -\pd{P}{x} - \frac{f}{D_h} \frac{\dot{m}|\dot{m}|}{2\rho A^2} - \sum_{j} K_j \delta(x-x_j) \frac{\dot{m}|\dot{m}|}{2 \rho A^2} + \rho \bar{g} \dot \hat{u} + \pd{P_p}{x} \,

!equation id=energy_conservation
A \rho c_p \pd{T}{t} + \dot{m} c_p \pd{T}{x} = q_w^{''} P_w \,

where

- $t$ is time,
- $x$ is the distance along the path,
- $\dot{m}$ is the mass flow rate,
- $A$ is the cross-sectional area,
- $P$ is the pressure,
- $f$ is the darcy friction factor,
- $D_h$ is the hydraulic diameter,
- $\rho$ is the density,
- $K_j$ is the $j^{th}$ minor loss coefficient,
- $\delta$ is the dirac delta function,
- $\bar{g}$ is the gravitational acceleration vector,
- $\hat{u}$ is the unit vector in the direction of flow,
- $\alpha$ is the angle of the flow path with respect to horizontal,
- $P_p$ is the pump pressure in the flow path, so $\pd{P_p}{x}$ is the pump pressure gain per unit length,
- $c_p$ is the specific isobaric heat capacity of the fluid,
- $T$ is the bulk fluid temperature,
- $q_w^{''}$ is the heat flux through the bounding surface(s), and
- $P_w$ is the wetted perimeter.

## Discretization

Integrating the above momentum equation along a 1D flow path consisting of $N$ segments results in the following equations, ignoring conservation of mass since it has been substituted in the conservation of momentum and energy equations already:

!equation id=discretized_momentum
\sum_{i=1}^{N} \frac{L_i}{A_i} \pd{\dot{m}}{t} = - \Delta P_c - \sum_{i=1}^{N} \frac{f_i L_i}{D_{h,i}} \frac{\dot{m}|\dot{m}|}{2 \rho_i A_i^2} - \sum_{i=1}^{N} K_i \frac{\dot{m}|\dot{m}|}{2 \rho_i A_i^2} - \sum_{i=1}^{N} \rho_i |\bar{g}| \Delta z_i + \Delta P_p \,

where

- $L_i$ is the length of segment $i$,
- $\Delta P_c$ is the characteristic pressure drop from inlet to outlet, $\Delta P_c = \int_x^{L_i} \pd{P}{x}\,dx$,
- $K_i$ is the sum of forms loss coefficients on this segment, $K_i \frac{\dot{m}|\dot{m}|}{2 \rho_i A_i^2} = \int_x^{L_i} \sum_{j} K_j \delta(x-x_j)\frac{\dot{m}|\dot{m}|}{2 \rho A^2}\,dx$,
- $\Delta z_i$ is the height difference between the inlet and the outlet of a segment, $\rho_i |\bar{g}| \Delta z_i = \int_x^{L_i} \rho \bar{g} \dot \hat{u}\,dx$, and
- $\Delta P_p$ is the total pump pressure increase over the flow path, $\Delta P_p = \int_x^{L_i} \pd{P_p}{x}\,dx$,

A core assumption of this model is that the flow path consists of N segments, that the segments may have different cross sections, but within each segment, the cross section is assumed to be approximately constant. For example, a segment may have elbows, support structures, etc. within it but the overall cross section shouldn't change significantly over the length of the segment. Other properties are permitted to be unique to each segment as well, including viscosity, surface roughness, height change, and length.

Input parameters are all given as functors, which enables flexibility in controllability, state changes in time, etc. However, caution should be exercised in changing parameters in time, as the parameters are assumed to be constant in time in the mathematical derivation. Hence, allowing the parameters to vary in time is intended only for long-running transients where the change over time is small, or for cases where the immediate transient effects of changing a parameter in time are unimportant to the phenomena of interest. This isn't the model to use for studying rapid transient effects.

To preserve proper friction force direction regardless of flow direction, $\dot{m}^2$ is replaced with $\dot{m}|\dot{m}|$ in the friction terms. The density $\rho_i$ of each segment is evaluated at that segment's temperature, so the gravity term accounts for natural convection and the friction and form losses reflect the local fluid state. Because pressure effects on properties are neglected, this results in a "globally compressible, locally incompressible" formulation that accounts for natural circulation but is otherwise incompressible in formulation.

A segment may optionally be partially or fully filled with a lighter fluid (e.g., a gas-filled segment in a draining problem) by supplying a reference liquid fluid properties object with density $\rho_{\text{ref},i}$. The volumetric velocity is then continuous along the path, $v_i = \dot{m} / (\rho_{\text{ref},i} A_i)$, so the mass flux in segment $i$ is $G_i = \rho_i v_i$, and the friction and form loss pressure drops become $\rho_i v_i |v_i| / 2$ times their coefficients. The inertia sum on the left-hand side of [!eqref](discretized_momentum) is weighted by $\max(\rho_i / \rho_{\text{ref},i}, 10^{-6})$, giving:

!equation id=weighted_inertia
\sum_{i=1}^{N} \frac{\rho_i}{\rho_{\text{ref},i}} \frac{L_i}{A_i} \pd{\dot{m}}{t} \,

such that light segments carry almost no inertia. Without a reference fluid, $\rho_{\text{ref},i} = \rho_i$ and the weighting is unity.

Given the effect of local temperature on the momentum equation, the area-averaged energy equation is discretized on a per-segment basis, meaning no path-integration is necessary. An upwinding finite-difference scheme is selected to compute the segment spatial derivative term as:

!equation id=upwind_temperature_derivative
\dot{m} c_p \pd{T}{x} = \frac{\dot{m}}{2 L} \left(1 - \frac{|\dot{m}|}{\dot{m}}\right) c_p T_d - \frac{\dot{m}}{2 L} \left(1 + \frac{|\dot{m}|}{\dot{m}}\right) c_p T_u + \frac{|\dot{m}| c_p}{L} T \,

where

- $T_d$ is the downstream segment's temperature, and
- $T_u$ is the upstream segment's temperature.

The full discretized energy equation is:

!equation id=discretized_energy
A \rho c_p \pd{T}{t} + \frac{\dot{m}}{2 L} \left(1 - \frac{|\dot{m}|}{\dot{m}}\right) c_p T_d - \frac{\dot{m}}{2 L} \left(1 + \frac{|\dot{m}|}{\dot{m}}\right) c_p T_u + \frac{|\dot{m}| c_p}{L} T = h P_w \left( T_w - T_m \right) \,

where

- $h$ is the heat transfer coefficient,
- $T_w$ is the wall temperature, and
- $T_m$ is the bulk fluid temperature seen by the wall.

By default the node temperature is used, $T_m = T$, which is monotone for any number of transfer units and any flow rate. Alternatively, the segment average may be selected, $T_m = (T + T_{\text{in}})/2$, with $T_{\text{in}} = \frac{1}{2} \left( 1 - \frac{|\dot{m}|}{\dot{m}} \right) T_d + \frac{1}{2} \left( 1 + \frac{|\dot{m}|}{\dot{m}} \right) T_u$ the upwinded inlet temperature. When a reference fluid is supplied, the advection terms are scaled by $\rho / \rho_{\text{ref}}$ (volumetric continuity).

The temperature of the segment is defined at the opposite end of the segment from the flow entrance. When the segment average is selected, proper conservation of energy therefore requires evaluating the wall heat transfer between $T_w$ and the mean of the segment's inlet and outlet temperatures, following the form of Newton's law of cooling. Furthermore, a modified advection term is used to ensure the proper directionality of the advection regardless of the flow direction.

Most closure quantities in the governing equations are user-supplied quantities, including: flow area, segment length, reference pressure, forms loss coefficients, gravitational acceleration, segment height changes $\Delta z_i$, pump pressure gains, surface roughness $\epsilon$, and wetted perimeters.

A few notable closure quantities are not user-specified inputs, and are worth some discussion.

The thermophysical properties ($\rho$, $\mu$, $k$, and $c_p$) are computed from the coupled temperature values and the reference pressure using a supplied [SinglePhaseFluidProperties.md] object. In the momentum equation, the properties of each segment are computed from that segment's temperature. Since the temperature equation is solved on a segment-local basis, local temperature is also used to compute the thermophysical properties there, with the exception of the bulk temperature used for wall heat transfer described below [!eqref](dittus_boelter).

The hydraulic diameter of each segment is computed using the closure relation:

!equation id=hydraulic_diameter
D_{h,i} = \frac{4 A_i}{P_{w,i}} \,

The friction factor of each segment is computed differently for laminar, turbulent, and transition flow, based on the Reynolds number $\text{Re} = |G| D_h / \mu$, with $G = \dot{m} / A_i \rho / \rho_{\text{ref}}$ as the segment mass flux and $\mu$ the dynamic viscosity. $\text{Re}$ is bounded below by $10^{-3}$ to avoid division by zero at zero flow. For $\text{Re} < 2300$ the flow is deemed laminar and the following analytical relation is used:

!equation id=laminar_friction
f = f_{\text{lam}}(\text{Re}) = \frac{64}{\text{Re}} \,

For $\text{Re} > 4000$ the flow is deemed turbulent and the Swamee-Jain approximation of the Colebrook-White equation is used [!citep](swamee_jain1976):

!equation id=swamee_jain_friction
f = f_{\text{turb}}(\text{Re}) = \frac{0.25}{\left( \log_{10} \left( \frac{\epsilon}{3.7 D_{h,i}} + \frac{5.74}{\text{Re}^{0.9}} \right) \right)^2} \,

where

- $\epsilon$ is the surface roughness.

For $2300 < \text{Re} < 4000$ the flow is deemed to be transitioning and a linear interpolation between the laminar prediction at $\text{Re} = 2300$ and the turbulent prediction at $\text{Re} = 4000$ is used:

!equation id=transition_friction
f = f_{\text{lam}}(2300) + \left(f_{\text{turb}}(4000) - f_{\text{lam}}(2300)\right) \frac{\text{Re} - 2300}{1700} \,

The heat transfer coefficient is computed using the Dittus-Boelter correlation [!citep](dittus1930heat):

!equation id=dittus_boelter
h = \max\left(0.023 \text{Re}^{0.8} Pr^{0.4},\, 4\right) \frac{k_f}{D_h} \,

where $Pr = c_p \mu / k_f$, $k_f$ is the fluid thermal conductivity. The lower bound of 4 on the Nusselt number is the approximate laminar limit, which keeps the heat transfer coefficient finite and physically reasonable at low flow. The fluid properties are evaluated at the segment film temperature, the mean of the segment fluid temperature and its upwinded inlet value, independent of the bulk temperature option $T_m$ used in the wall term of [!eqref](discretized_energy). For
the conjugate heat transfer kernels below, this coefficient is supplied by
[ADDittusBoelterHTCFunctorMaterial.md].

## Junctions

The above equations are valid for 1D flow paths only; for a flow junction, the mass conservation equation is as follows:

!equation id=junction_mass_conservation
\dot{m}_{\text{in}} = \dot{m}_{out} \,

Typically, it is sufficient to assume that negligible momentum and energy transfer occurs at a junction.

## Conjugate Heat Transfer

The flow energy equation above is designed to couple to a variable segment wall temperature. For convenience and modularity, additional conjugate heat transfer kernels are added to close the heat transfer through the solid bounding wall. The governing equation for which is the heat conduction equation in the solid wall:

!equation id=pipe_CHT_conservation
\rho_w c_{p,w} \pd{T_w}{t} = \nabla \cdot \left (k_w \nabla T_w \right )

where

- $\rho_w$ is the density of the wall,
- $c_{p,w}$ is the specific isobaric heat capacity of the wall,
- $T_w$ is the temperature of the wall, and
- $k_w$ is the thermal conductivity of the wall.

The heat flux entering the inner surface of the wall and the heat flux exiting the outer surface of the wall enter as boundary conditions of this equation. Integrating it over the wall cross-section and over each segment, on a per-segment basis to match what is done in [!eqref](discretized_energy), yields the node equations below. Furthermore, the wall is discretized into two nodes through the thickness to capture the inner wall temperature and the outer wall temperature, as shown in [fig:pipe_CHT_discretization].

!media modules/thermal_hydraulics/media/conjugate-heat-transfer_pipe_cross_section.png
       id=fig:pipe_CHT_discretization
       caption=Conjugate heat transfer illustration for a simple pipe.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;text-align:center;

Thus, the discretized energy equation for the inner wall temperature is:

!equation id=discretized_inner_pipe_temperature
A_{i,w} \rho_w c_{p,w} \pd{T_{i,w}}{t} = \frac{k_w P_{m,w} \left( T_{o,w} - T_{i,w} \right)}{\delta} +
\frac{G_{i,u} \left( T_{i,u} - T_{i,w} \right) + G_{i,d} \left( T_{i,d} - T_{i,w} \right)}{L} +
h P_{i,w} \left( T_m - T_{i,w} \right)

where

- $A_{i,w}$ is the cross-sectional area of the inner layer of the wall,
- $T_{i,w}$ is the temperature of the inner surface of the wall,
- $T_{o,w}$ is the temperature of the outer surface of the wall,
- $P_{m,w}$ is the perimeter of the mid-thickness dividing line that represents the boundary between the inner layer and the outer layer of the wall,
- $\delta$ is the thickness of the wall,
- $L$ is the segment length,
- $h$ is the heat transfer coefficient of the fluid,
- $T_m$ is the fluid bulk temperature defined for [!eqref](discretized_energy), which must use the same option as the fluid energy equation so that the wall and fluid exchange the same heat.

$G_{i,u}$ and $G_{i,d}$ are the axial face conductances to the upstream and downstream inner wall nodes. Each is the harmonic mean of two half-length conduction resistances in series across the face, accounting for the upstream/downstream node potentially having a different cross-section than this node:

!equation id=upstream_axial_conductance
G_{i,u} = \left( \frac{\Delta x_u}{2 k_w A_{i,w}} + \frac{\Delta x_u}{2 k_u A_{i,u}} \right)^{-1}

!equation id=downstream_axial_conductance
G_{i,d} = \left( \frac{\Delta x_d}{2 k_w A_{i,w}} + \frac{\Delta x_d}{2 k_d A_{i,d}} \right)^{-1}

where

- $\Delta x_u$ and $\Delta x_d$ are the axial spacings to the upstream and downstream nodes, supplied as inputs (for node-centered control volumes, $\Delta x_u = L/2 + L_u/2$),
- $k_u$ and $k_d$ are the wall conductivity evaluated at the upstream and downstream node's own temperature, and
- $A_{i,u}$ and $A_{i,d}$ are the layer area evaluated at the upstream and downstream node's own cross-section.

Similarly, the discretized energy equation for the outer wall temperature is:

!equation id=discretized_outer_pipe_temperature
A_{o,w} \rho_w c_{p,w} \pd{T_{o,w}}{t} = \frac{k_w P_{m,w} \left( T_{i,w} - T_{o,w} \right)}{\delta} +
\frac{G_{o,u} \left( T_{o,u} - T_{o,w} \right) + G_{o,d} \left( T_{o,d} - T_{o,w} \right)}{L} + q_o^{'}

where

- $A_{o,w}$ is the cross-sectional area of the outer layer of the wall,
- $T_{o,u}$ and $T_{o,d}$ are the upstream and downstream outer-surface wall temperatures,
- $G_{o,u}$ and $G_{o,d}$ are the axial face conductances of the outer layer, defined as $G_{i,u}$ and $G_{i,d}$ above but with the outer-layer areas and node temperatures, and
- $q_o^{'}$ is the per-length heat transfer term through the outer boundary.

Since the outer boundary of the wall may be subject to a number of boundary conditions, a number of kernels are provided for flexibility in modeling the per-length heat transfer term. The base kernels include:

- Ambient convection: [ADPipeOuterWallAmbientTemperatureScalarKernel.md]
- Coupled secondary-side convection: [ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel.md]
- Applied heat flux: [ADPipeOuterWallHeatFluxScalarKernel.md]

An optional standalone term for radiative heat transfer [ADPipeOuterWallRadiationScalarKernel.md] may also be added to any of the base outer wall temperature kernels (but it really only makes sense for the ambient and coupled convection kernels).
