# Water-steam equation of state

A single component two-phase model suitable for non-isothermal water-steam transport is available in PorousFlow based on the [persistent variable](/persistent_variables.md) approach.

The primary variables for this model are porepressure of the liquid phase and enthalpy, as they form a complete set of persistent variables. Temperature and vapor saturation are then calculated using these primary variables.

!alert note
This fluid state model must be run in a non-isothermal setting. If an isothermal model of
water flow is required, just use one of the [single phase](singlephase.md) models instead.

The temperature as a function of water porepressure and enthalpy is shown in [fig:phase_diagram] for the entire range of pressure and temperature that this equation of state is valid for. In the two-phase region (inside the phase envelope shown in [fig:phase_diagram]), the water temperature
is constant for a given pressure along the saturation curve as the phase transitions from a liquid (the region outside and to the left of the phase envelope) to a vapor (the region outside and to the right of the phase envelope). It is for this reason that pressure and temperature cannot be used as the primary variables, as they are not independent in this two-phase region.

!media media/porous_flow/water_phase_diagram_ph.png
       id=fig:phase_diagram
       style=width:80%;margin-left:10px;
       caption=Temperature of water as a function of porepressure and enthalpy. Phase envelope
       shown as solid line. (a) Full parameter space; (b) Vicinity of two-phase region. Isotherms shown as dashed lines.

The left-hand side of the phase envelope corresponds to a fully saturated liquid phase, while the right-hand side corresponds to a fully saturated vapor phase. In between, both liquid and vapor phases coexist, with the composition described by the vapor quality

\begin{equation}
X = \frac{m_v}{m_l + m_v},
\label{eq:quality}
\end{equation}

where $m_v$ and $m_l$ are the mass in the vapor and liquid phase, respectively. A fully saturated liquid phase therefore has a quality of 0 and a fully saturated vapor has a quality of 1.

Alternatively, vapor quality can be expressed in terms of enthalpy for a single-component system (e.g. water and water vapor)

\begin{equation}
X = \frac{h - h_l}{h_v - h_l},
\label{eq:hq}
\end{equation}

where $h$ is the total enthalpy, and $h_v$ and $h_l$ are the enthalpies for fully saturated vapor and liquid phases, respectively. The quantity $h_v - h_l$ is the enthalpy of vaporization, and corresponds to the enthalpy difference along an isotherm in the two-phase region (the enthalpies on the phase envelope at a given pressure).

The mass of phase $\alpha$ is defined as $m_{\alpha} = \phi S_{\alpha} \rho_{\alpha}$ where $\phi$ is the porosity of the porous media, $S_{\alpha}$ is the saturation of phase $\alpha$ and $\rho_{\alpha}$ its density. Using this definition and [eq:hq], the saturation of the vapor phase in the two-phase region can be calculated as

\begin{equation}
S_v = \frac{h \rho_l - h_l \rho_l}{h_v \rho_v - h_l \rho_l + h(h_v - h_l)}.
\label{eq:satv}
\end{equation}

## Thermophysical properties

In the single phase liquid or vapor regions, all water properties are calculated by first calculating temperature given porepressure and enthalpy (except for enthalpy, of course, which is a nonlinear variable in this case).

In the two-phase region, the properties of the liquid and vapor are calculated at the saturation temperature and pressure, with enthalpy given by the fully-saturated state for either vapor or liquid.

## Implementation

### Variables

This class requires pressure of the liquid phase and enthalpy as primary variables.

!listing modules/porous_flow/test/tests/fluidstate/water_vapor.i block=Variables

!alert note
These variables must be listed as PorousFlow variables in the PorousFlowDictator UserObject

In the simplest case, the number of fluid components should be set to one, and the number of phases set to two in the [PorousFlowDictator](/PorousFlowDictator.md).

!listing modules/porous_flow/test/tests/fluidstate/water_vapor.i block=UserObjects/dictator

It can be beneficial to scale the residual contribution for each variable to assist nonlinear convergence. Following the discussion about [convergence](/porous_flow/convergence.md) in the PorousFlow documentation, the residual contribution from the fluid heat equations is approximately 1,000 times larger than the residual contribution from the fluid mass equations.

### UserObjects

This fluid state is implemented in the [`PorousFlowWaterVapor`](/PorousFlowWaterVapor.md)
UserObject. This UserObject is a `GeneralUserObject` that contains methods that provide a
complete thermophysical description of the model given pressure and enthalpy.

!listing modules/porous_flow/test/tests/fluidstate/water_vapor.i block=UserObjects/fs

The water fluid formulation is set in the [FluidProperties](/fluid_properties/index.md) block

!listing modules/porous_flow/test/tests/fluidstate/water_vapor.i block=FluidProperties

### Materials

The [`PorousFlowFluidStateSingleComponent`](/PorousFlowFluidStateSingleComponent.md) material provides all phase pressures, saturation, densities, viscosities etc using the formulation provided in the [`PorousFlowWaterVapor`](/PorousFlowWaterVapor.md) UserObject.

!listing modules/porous_flow/test/tests/fluidstate/water_vapor.i block=Materials/watervapor

## Initial condition

An initial condition that makes it easy to set enthalpy for a specified pressure and temperature (which may be more suitable for the user) is provided in [PorousFlowFluidPropertyIC](/PorousFlowFluidPropertyIC.md).

!listing modules/porous_flow/test/tests/ics/fluidpropic.i block=ICs
