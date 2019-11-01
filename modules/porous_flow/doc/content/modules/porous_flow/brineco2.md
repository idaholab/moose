# Brine and carbon dioxide

The brine-CO$_2$ model available in PorousFlow is a high precision equation of state
for brine and CO$_2$, including the mutual solubility of CO$_2$ into the liquid brine
and water into the CO$_2$-rich gas phase. This model is suitable for simulations of
geological storage of CO$_2$ in saline aquifers, and is valid for temperatures in the range
$12^{\circ}C \le T \le 300^{\circ}C$ and pressures less than 60 MPa.

Salt (NaCl) can be included as a nonlinear variable, allowing salt to be transported
along with water to provide variable density brine.

## Phase composition

The mass fractions of CO$_2$ in the liquid phase and H$_2$O in the gas phase are calculated
using the accurate fugacity-based formulation of [!cite](spycher2003) and [!cite](spycher2005)
for temperatures below 100$^{\circ}$C, and the elevated temperature formulation of
[!cite](spycher2010) for temperatures above 110$^{\circ}$C.

As these formulations do not coincide for temperatures near 100$^{\circ}$C, a cubic
polynomial is used to join the two curves smoothly, see [soltemp] for an example:

!media media/porous_flow/solubility_temperature.png
       id=soltemp
       style=width:60%;margin-left:10px;
       caption=Dissolved CO$_2$ mass fraction in brine versus temperature showing the low
       and high temperature formulations joined smoothly by a cubic polynomial. Results for
       pressure of 10 MPa and salt mass fraction of 0.01

!alert note
The mutual solubilities in the elevated temperature regime must be calculated iteratively,
so an increase in computational expense can be expected.

This is similar to the formulation provided in the ECO2N module of TOUGH2 [!citep](pruess1999),
see [xco2l] and [yh2og] for a comparison between the two codes.

!media media/porous_flow/brineco2_xco2l.png
       id=xco2l
       style=width:60%;margin-left:10px;
       caption=Dissolved CO$_2$ mass fraction in brine - comparison between MOOSE and TOUGH2
       at 30$^{\circ}$C

!media media/porous_flow/brineco2_yh2og.png
      id=yh2og
      style=width:60%;margin-left:10px;
      caption=H$_2$O mass fraction in gas - comparison between MOOSE and TOUGH2
      at 30$^{\circ}$C

As with many reservoir simulators, it is assumed that the phases are in instantaneous
equilibrium, meaning that dissolution of CO$_2$ into the brine and evaporation of water
vapor into the CO$_2$ phase happens instantaneously.

If the amount of CO$_2$ present is not sufficient to saturate the brine, then no gas
phase will be formed, and the dissolved CO$_2$ mass fraction will be smaller than its
equilibrium value. Similarly, if the amount of water vapor present is smaller than its
equilibrium value in the gas phase, then no liquid phase can be formed.

If there is sufficient CO$_2$ present to saturate the brine, then both liquid and gas
phases will be present. In this case, the mass fraction of CO$_2$ dissolved in the brine
is given by its equilibrium value (for the current pressure, temperature and salinity). Similarly, the mass fraction of water vapor in the gas phase will be given by its
equilibrium value in this two phase region.

## Thermophysical properties

### Density

The density of the aqueous phase with the contribution of dissolved CO$_2$ is calculated using
\begin{equation}
\frac{1}{\rho} = \frac{1 - X_{CO2}}{\rho_b} + \frac{X_{CO2}}{\rho_{CO2}},
\end{equation}
where $\rho_b$ is the density of brine (supplied using a
[`BrineFluidProperties`](/BrineFluidProperties.md) UserObject), $X_{CO2}$ is the
mass fraction of CO$_2$ dissolved in the aqueous phase, and $\rho_{CO2}$ is the partial
density of dissolved CO$_2$ [!citep](garcia2001).

As water vapor is only ever a small component of the gas phase in the temperature and pressure ranges
that this class is valid for, the density of the gas phase is assumed to be simply the density of CO$_2$
at the given pressure and temperature, calculated using a [`CO2FluidProperties`](/CO2FluidProperties.md)
UserObject.

### Viscosity

No contribution to the viscosity of each phase due to the presence of CO$_2$ in the aqueous
phase or water vapor in the gas phase is included. As a result, the viscosity of the aqueous
phase is simply the viscosity of brine, while the viscosity of the gas phase is the viscosity
of CO$_2$.

### Enthalpy

The enthalpy of the gas phase is simply the enthalpy of CO$_2$ at the given pressure and temperature
values, with no contribution due to the presence of water vapor included.

The enthalpy of the liquid phase is also calculated as a weighted sum of the enthalpies
of each individual component, but also includes a contribution due to the enthalpy of
dissolution (a change in enthalpy due to dissolution of the NCG into the water phase)
\begin{equation}
h_l = (1 - X_{CO2}) h_b + X_{CO2} h_{CO2,aq},
\end{equation}
where $h_b$ is the enthalpy of brine, and $h_{CO2,aq}$ is the enthalpy of CO$_2$ in the liquid
phase, which includes the enthalpy of dissolution $h_{dis}$
\begin{equation}
h_{CO2,aq} = h_{CO2} + h_{dis}.
\end{equation}

In the range of pressures and temperatures considered, CO$_2$ may exist as a gas or a supercritical fluid. Using a linear fit to the model of [!cite](duan2003), the enthalpy of
dissolution of both gas phase and supercritical CO$_2$ is calculated as
\begin{equation}
h_{dis}(T) = \frac{-58353.3 + 134.519 T}{M_{CO2}}.
\end{equation}

The calculation of brine and CO$_2$ fluid properties can take a significant proportion of each simulation,
so it is suggested that tabulated versions of both CO$_2$ and brine proeprties are used using
[TabulatedFluidProperties](/TabulatedFluidProperties.md).

## Implementation

### Variables

This class requires pressure of the gas phase, the total mass fraction of CO$_2$ summed over all
phases (see [compositional flash](/compositional_flash.md) for details), and the mass fraction
of salt in the brine, for isothermal simulations. For nonisothermal simulations, temperature must
also be provided as a nonlinear variable.

!alert note
These variables must be listed as PorousFlow variables in the PorousFlowDictator UserObject

In the general isothermal case, three variables (gas porepressure, total CO$_2$ mass fraction and salt mass fraction)
are provided, one for each of the three possible fluid components (water, salt and CO$_2$). As a result, the
number of components in the PorousFlowDictator must be set equal to three.

!listing modules/porous_flow/test/tests/fluidstate/theis_brineco2.i block=UserObjects/dictator

For nonisothermal cases, temperature must also be provided to ensure that all of the derivatives with
respect to the temperature variable are calculated.

!listing modules/porous_flow/test/tests/fluidstate/theis_brineco2_nonisothermal.i block=UserObjects/dictator

Optionally, the salt mass fraction can be treated as a constant, in which case only gas porepressure and
total CO$_2$ mass fraction are required for the isothermal case, and the number of fluid components is two.

!listing modules/porous_flow/test/tests/fluidstate/brineco2.i block=UserObjects/dictator

### UserObjects

This fluid state is implemented in the [`PorousFlowBrineCO2`](/PorousFlowBrineCO2.md) UserObject.
This UserObject is a `GeneralUserObject` that contains methods that provide a complete
thermophysical description of the model given pressure, temperature and salt mass fraction.

!listing modules/porous_flow/test/tests/fluidstate/brineco2.i block=UserObjects/fs

### Materials

The [`PorousFlowFluidState`](/PorousFlowFluidState.md) material provides all phase pressures, saturation, densities, viscosities etc, as well as all mass fractions of all fluid components in all fluid phases in a single material using the formulation provided in the [`PorousFlowBrineCO2`](/PorousFlowBrineCO2.md) UserObject.

!listing modules/porous_flow/test/tests/fluidstate/brineco2.i block=Materials/brineco2

For nonisothermal simulations, the temperature variable must also be supplied to ensure that
all the derivatives with respect to the temperature variable are computed for the Jacobian.

!listing modules/porous_flow/test/tests/fluidstate/theis_brineco2_nonisothermal.i block=Materials/brineco2

## Initial condition

The nonlinear variable representing CO$_2$ is the total mass fraction of CO$_2$ summed over
all phases. In some cases, it may be preferred to provide an initial CO$_2$ saturation, rather
than total mass fraction. To allow an initial saturation to be specified, the
[`PorousFlowFluidStateIC`](/PorousFlowFluidStateIC.md) initial
condition is provided. This initial condition calculates the total mass fraction of CO$_2$
summed over all phases for a given saturation.

!listing modules/porous_flow/test/tests/fluidstate/brineco2_ic.i block=ICs

## Example

### 1D Radial injection

Injection into a one dimensional radial reservoir is a useful test of this class, as it
admits a similarity solution $\zeta = r^2/t$, where $r$ is the radial distance from the
injection well, and $t$ is time. This similarity solution holds even when complicated physics
such as mutual solubility is included.

An example of a 1D radial injection problem is included in the test suite.

!listing modules/porous_flow/test/tests/fluidstate/theis_brineco2.i

Initially, all of the CO$_2$ injected is dissolved in the resident brine, as the amount
of CO$_2$ is less than the dissolved mass fraction at equilibrium. After a while, the
brine near the injection well becomes saturated with CO$_2$, and a gas phase forms. This
gas phase then spreads radially as injection continues. The mass fraction of salt in the brine  
decreases slightly where CO$_2$ is present, due to the increased density of the brine following
dissolution of CO$_2$.

A comparison of the results expressed in terms of the similarity solution is presented in
[theis] for all three nonlinear variables for the case where $r$ is fixed (evaluated using
Postprocessors), and also the case where $t$ is fixed (evaluated using a VectorPostprocessor).

!media media/porous_flow/theis_similarity_brineco2.png
       id=theis
       style=width:60%;margin-left:10px;
       caption=Similarity solution for 1D radial injection example

As these results show, the similarity solution holds for this problem.

An nonisothermal example of the above problem, where cold CO$_2$ is injected into a warm aquifer,
is also provided in the test suite.

!listing modules/porous_flow/test/tests/fluidstate/theis_brineco2_nonisothermal.i

!bibtex bibliography
