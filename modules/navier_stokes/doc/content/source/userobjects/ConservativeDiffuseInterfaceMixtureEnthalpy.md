# ConservativeDiffuseInterfaceMixtureEnthalpy

!syntax description /UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy

## Description

`ConservativeDiffuseInterfaceMixtureEnthalpy` relates the mixture enthalpy density $\rho h$ of a
two-phase diffuse-interface flow to its temperature $T$:

\begin{equation}
\rho h = \alpha \rho_l h_l(T) + (1 - \alpha) \rho_g h_g(T),
\end{equation}

where $\alpha$ is the liquid
[!param](/UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy/volume_fraction), $\rho_l$ and
$\rho_g$ are the constant
[!param](/UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy/liquid_density) and
[!param](/UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy/gas_density), and $h_l$ and
$h_g$ are the specific enthalpies of
[!param](/UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy/liquid_fluid_properties) and
[!param](/UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy/gas_fluid_properties) at the
[!param](/UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy/thermodynamic_pressure).

It does three things:

- At the start of the simulation, it sets
  [!param](/UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy/enthalpy_variable) from the
  initial [!param](/UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy/temperature) and
  volume fraction. A recovered simulation restores $\rho h$ from the checkpoint instead.
- After each energy solve, it recovers the temperature in every cell by Newton iteration on the
  relation above, starting from the previous temperature. It stops with an error if a cell does
  not converge.
- It provides the energy kernels with the linearization of $\rho h$ about the lagged temperature
  $T^*$ of a cell, $T = T^* + \left( \rho h - (\rho h)^* \right) / C$, with the mixture heat
  capacity $C = \sum_k \alpha_k \rho_k c_{p,k}(T^*)$. It also provides the cell thermal
  conductivity, the series mix $1/k = \sum_k \alpha_k / k_k$.

The series mix makes the cell resistances across a diffuse interface add up to those of the two
sharp layers, however many cells the interface spreads over.

This object is created by
[WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics](WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics.md),
which also creates the kernels and boundary conditions that use it.

## Example Input Syntax

In this example, the heat transfer physics creates this object for the conserved enthalpy
`rho_h` and the temperature `T` of a liquid layer under a gas layer. The two phases have different
conductivities and heat capacities that vary with temperature, given by the fluid properties
objects `fp_l` and `fp_g`.

!listing modules/navier_stokes/test/tests/finite_volume/two_phase/diffuse_interface/two_layer_conduction.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceHeatTransferSegregated

!syntax parameters /UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy

!syntax inputs /UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy

!syntax children /UserObjects/ConservativeDiffuseInterfaceMixtureEnthalpy
