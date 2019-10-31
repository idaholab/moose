# Water and non-condensable gas

A miscible water and non-condensable gas (NCG) model is available in PorousFlow based on the [persistent variable](/persistent_variables.md) approach.

## Available NCG's

Any of the gaseous fluids provided in the [Fluid Properties](/fluid_properties/index.md) module
can be used in this equation of state.

## Phase composition

Henry's law is used to calculate the mass fraction of the NCG in the water phases
\begin{equation}
x_{ncg} = \frac{P_{ncg}}{K_H},
\end{equation}
while Raoult's law is used to calculate the mass fraction of H$_2$O in the gas phase
\begin{equation}
y_{h2o} = \frac{P_{\mathrm{sat}}}{P},
\end{equation}
where $P_{\mathrm{sat}}$ is vapor pressure of water.

As with many reservoir simulators, it is assumed that the phases are in instantaneous
equilibrium, meaning that dissolution of NCG into the water and evaporation of water
vapor into the gas phase happens instantaneously.

If the amount of NCG present is not sufficient to saturate the water, then no gas
phase will be formed, and the dissolved NCG mass fraction will be smaller than its
equilibrium value. Similarly, if the amount of water vapor present is smaller than its
equilibrium value in the gas phase, then no liquid phase can be formed.

If there is sufficient NCG present to saturate the water, then both liquid and gas
phases will be present. In this case, the mass fraction of NCG dissolved in the water
is given by its equilibrium value. Similarly, the mass fraction of water vapor in the
gas phase will be given by its equilibrium value in this two phase region.

## Thermophysical properties

### Density

The density of the gas phase is calculated as the sum of the densities of each fluid
component at their partial pressure (via Dalton's law).

\begin{equation}
\rho_g = \rho_{ncg} + \rho_{vap}.
\end{equation}

No change to the water density is
calculated for dissolved NCG components.

### Viscosity

The viscosity of the gas phase $\mu_g$ is given by a weighted sum of the viscosities of each
fluid component

\begin{equation}
\mu_g = y_{ncg} \mu_{ncg} + (1 - y_{ncg}) \mu_{h2o}.
\end{equation}

The viscosity of the liquid phase is simply given by the viscosity of water - no contribution
due to dissolved NCG is included.

### Enthalpy

The enthalpy of the gas phase is calculated as a weighted sum of the enthalpies of each
fluid component

\begin{equation}
h_g = y_{ncg} h_{ncg} + (1 - y_{ncg}) h_{h2o}.
\end{equation}

The enthalpy of the liquid phase is also calculated as a weighted sum of the enthalpies
of each individual component, but also includes a contribution due to the enthalpy of
dissolution (a change in enthalpy due to dissolution of the NCG into the water phase)

\begin{equation}
h_l = (1 - x_{ncg}) h_{h2o} + x_{ncg} (h_{ncg,g} + h_{dis,g}),
\end{equation}

where $h_{ncg,g}$ is the enthalpy of the gaseous NCG, and $h_{dis,g}$ is the enthalpy of
dissolution of NCG into the liquid. An expression for the enthalpy of dissolution is
implemented following [!citep](himmelblau1959)

\begin{equation}
\frac{\partial \ln(K_h)}{\partial T} = - \frac{h_{dis,g} M_{ncg}}{R T^2},
\end{equation}

where $M_{ncg}$ is the molar mass of the NCG component, and $R$ is the universal
gas constant.

## Implementation

### Variables

This class requires pressure of the gas phase and the total mass fraction of NCG summed over all
phases (see [compositional flash](/compositional_flash.md) for details) for isothermal simulations.
For nonisothermal simulations, the temperature must also be provided as a nonlinear variable.

!alert note
These variables must be listed as PorousFlow variables in the PorousFlowDictator UserObject

In the isothermal case, two variables (gas porepressure and total NCG mass fraction) are required. The number of components in the PorousFlowDictator must be set equal to two.

!listing modules/porous_flow/test/tests/fluidstate/theis.i block=UserObjects/dictator

In the nonisothermal case, three variables (gas porepressure, total NCG mass fraction and temperature) are required. The number of components in the PorousFlowDictator is still two.

!listing modules/porous_flow/test/tests/fluidstate/theis_nonisothermal.i block=UserObjects/dictator

### UserObjects

This fluid state is implemented in the [`PorousFlowWaterNCG`](/PorousFlowWaterNCG.md)
UserObject. This UserObject is a `GeneralUserObject` that contains methods that provide a
complete thermophysical description of the model given pressure, temperature and total NCG
mass fraction.

!listing modules/porous_flow/test/tests/fluidstate/theis.i block=UserObjects/fs

Swapping NCG's is as simple as changing the `gas_fp` parameter in this UserObject!

### Materials

The [`PorousFlowFluidState`](/PorousFlowFluidState.md) material provides all phase pressures, saturation, densities, viscosities etc, as well as all mass fractions of all fluid components in all fluid phases in a single material using the formulation provided in the [`PorousFlowWaterNCG`](/PorousFlowWaterNCG.md) UserObject.

For isothermal simulations, this material will look like

!listing modules/porous_flow/test/tests/fluidstate/theis.i block=Materials/waterncg

For nonisothermal simulations, the temperature variable must also be supplied to ensure that
all the derivatives with respect to the temperature variable are computed for the Jacobian.

!listing modules/porous_flow/test/tests/fluidstate/theis_nonisothermal.i block=Materials/waterncg

## Initial condition

The nonlinear variable representing NCG is the total mass fraction of NCG summed over
all phases. In some cases, it may be preferred to provide an initial NCG saturation, rather
than total mass fraction. To allow an initial saturation to be specified, the
[`PorousFlowFluidStateIC`](/PorousFlowFluidStateIC.md) initial
condition is provided. This initial condition calculates the total mass fraction of NCG
summed over all phases for a given saturation.

!listing modules/porous_flow/test/tests/fluidstate/waterncg_ic.i block=ICs

## Example

### 1D Radial injection

Injection into a one dimensional radial reservoir is a useful test of this class, as it
admits a similarity solution $\zeta = r^2/t$, where $r$ is the radial distance from the
injection well, and $t$ is time. This similarity solution holds even when complicated physics
such as mutual solubility is included.

An example of an isothermal 1D radial injection problem is included in the test suite with CO$_2$ as the
NCG.

!listing modules/porous_flow/test/tests/fluidstate/theis.i

Initially, all of the CO$_2$ injected is dissolved in the resident water, as the amount
of CO$_2$ is less than the dissolved mass fraction at equilibrium. After a while, the
water near the injection well becomes saturated with CO$_2$, and a gas phase forms. This
gas phase then spreads radially as injection continues.

A comparison of the results expressed in terms of the similarity solution is presented in
[theis] for both nonlinear variables for the case where $r$ is fixed (evaluated using
Postprocessors), and also the case where $t$ is fixed (evaluated using a VectorPostprocessor).

!media media/porous_flow/theis_similarity_waterncg.png
       id=theis
       style=width:80%;margin-left:10px;
       caption=Similarity solution for 1D radial injection example

An nonisothermal example of the above problem, where cold CO$_2$ is injected into a warm aquifer,
is also provided in the test suite.

!listing modules/porous_flow/test/tests/fluidstate/theis_nonisothermal.i

!bibtex bibliography
