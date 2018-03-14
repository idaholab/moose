# Water and non-condensable gas

[`PorousFlowFluidStateWaterNCG`](/PorousFlowFluidStateWaterNCG.md)

A miscible water and non-condensable gas model is available in PorousFlow based on the [persistent variable](/persistent_variables.md) approach.

Henry's law is used to calculate the mass fraction of the NCG in the water phases
\begin{equation}
x_{NCG} = \frac{P_{NCG}}{K_H},
\end{equation}
while Raoult's law is used to calculate the mass fraction of H$_2$O in the gas phase
\begin{equation}
y_{H2O} = \frac{P_{\mathrm{sat}}}{P},
\end{equation}
where $P_{\mathrm{sat}}$ is vapor pressure of water.

The density of the gas phase is calculated as the sum of the densities of each fluid
component calculated using their partial pressure (via Dalton's law).

The [`PorousFlowFluidStateWaterNCG`](/PorousFlowFluidStateWaterNCG.md)
`Material` provides all phase pressures, saturation, densities, viscosities etc, as well
as all mass fractions of all fluid components in all fluid phases in a single material.

To allow an initial saturation to be specified, the
[`PorousFlowFluidStateWaterNCGIC`](/PorousFlowFluidStateWaterNCGIC.md) initial
condition is provided. This initial condition calculates the total mass fraction of NCG
summed over all phases for a given saturation.
