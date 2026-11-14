# LinearFVConservativeDiffuseInterfaceEnthalpyBC

!syntax description /LinearFVBCs/LinearFVConservativeDiffuseInterfaceEnthalpyBC

## Description

`LinearFVConservativeDiffuseInterfaceEnthalpyBC` is the boundary condition on the mixture enthalpy
density $\rho h$ of a two-phase diffuse-interface flow. It takes either a
[!param](/LinearFVBCs/LinearFVConservativeDiffuseInterfaceEnthalpyBC/temperature) or a
[!param](/LinearFVBCs/LinearFVConservativeDiffuseInterfaceEnthalpyBC/heat_flux) into the domain,
not both. Without a temperature, the heat flux applies, and its default of zero makes the boundary
adiabatic.

With a temperature $T_b$, the boundary value of $\rho h$ is that of $T_b$ at the face
[!param](/LinearFVBCs/LinearFVConservativeDiffuseInterfaceEnthalpyBC/volume_fraction), as given
by the [!param](/LinearFVBCs/LinearFVConservativeDiffuseInterfaceEnthalpyBC/mixture_enthalpy)
object. The energy kernels then conduct heat from the boundary temperature and let entering fluid
carry its enthalpy. Without a temperature, the boundary value is that of the adjacent cell: fluid
crossing the boundary in either direction carries the temperature of the cell.

This is the only boundary condition accepted by
[LinearFVConservativeDiffuseInterfaceEnthalpyAdvection](LinearFVConservativeDiffuseInterfaceEnthalpyAdvection.md)
and
[LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion](LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion.md).
[WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics](WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics.md)
creates one on every inlet, wall and outlet of the coupled flow: with a temperature for
`fixed-temperature` inlets and walls, with a heat flux for `heatflux` walls, and with a zero heat
flux on outlets.

## Example Input Syntax

In this example, the heat transfer physics creates this boundary condition on the four walls of a
differentially heated cavity: fixed temperatures on the hot and cold walls, and a zero heat flux
on the top and bottom.

!listing modules/navier_stokes/test/tests/finite_volume/two_phase/diffuse_interface/differentially_heated_cavity.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceHeatTransferSegregated

!syntax parameters /LinearFVBCs/LinearFVConservativeDiffuseInterfaceEnthalpyBC

!syntax inputs /LinearFVBCs/LinearFVConservativeDiffuseInterfaceEnthalpyBC

!syntax children /LinearFVBCs/LinearFVConservativeDiffuseInterfaceEnthalpyBC
