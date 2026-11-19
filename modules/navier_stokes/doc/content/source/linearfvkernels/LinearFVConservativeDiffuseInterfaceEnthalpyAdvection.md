# LinearFVConservativeDiffuseInterfaceEnthalpyAdvection

!syntax description /LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyAdvection

## Description

`LinearFVConservativeDiffuseInterfaceEnthalpyAdvection` adds the advection term of the equation
for the mixture enthalpy density $\rho h$ of a two-phase diffuse-interface flow. Each phase carries
its own specific enthalpy with its own mass flux, upwinded separately:

\begin{equation}
\int_{\partial V} \sum_k \dot{m}_k h_k(T_{up,k}) \, dS,
\end{equation}

where $T_{up,k}$ is the temperature of the cell upwind of the face for the mass flux $\dot{m}_k$
of phase $k$. The liquid mass flux is $\dot{m}_l = \rho_l F_\alpha$, with the area-integrated
limited volume-fraction flux $F_\alpha$ of
[!param](/LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyAdvection/volume_fraction_flux).
The gas mass flux is $\dot{m}_g = F_\rho - \dot{m}_l$, with the mixture mass flux $F_\rho$ of
[!param](/LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyAdvection/mass_flux).
Both fluxes are published by the volume-fraction corrector of the coupled diffuse-interface
physics, so the energy moves with exactly the mass of each phase that advanced the volume
fraction.

The phase enthalpy is linearized in $\rho h$ through the cell linearization of the
[!param](/LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyAdvection/mixture_enthalpy)
object, $T = T^* + \left( \rho h - (\rho h)^* \right) / C$, which makes the term a deferred
correction that is exact once the outer iterations converge.

On boundaries, fluid leaving the domain carries the enthalpy of the cell. Fluid entering through
a boundary with a prescribed temperature carries the enthalpy of that temperature; otherwise it
carries the enthalpy of the cell. The kernel only accepts
[LinearFVConservativeDiffuseInterfaceEnthalpyBC](LinearFVConservativeDiffuseInterfaceEnthalpyBC.md)
boundary conditions on its variable.

This kernel is created by
[WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics](WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics.md).

## Example Input Syntax

In this example, the heat transfer physics creates this kernel for a slab of liquid advected
through gas at a uniform temperature. The phases have different enthalpies that depend
nonlinearly on temperature, and the phase-wise advection keeps the temperature uniform while the
slab moves.

!listing modules/navier_stokes/test/tests/finite_volume/two_phase/diffuse_interface/uniform_temperature_advection.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceHeatTransferSegregated

!syntax parameters /LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyAdvection

!syntax inputs /LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyAdvection

!syntax children /LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyAdvection
