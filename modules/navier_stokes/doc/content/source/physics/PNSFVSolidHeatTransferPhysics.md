# Navier Stokes Solid Heat Transfer / PNSFVSolidHeatTransfer

!syntax description /Physics/NavierStokes/SolidHeatTransfer/PNSFVSolidHeatTransferPhysics

## Automatically created variables

The `PNSFVSolidHeatTransferPhysics` by default will automatically create a nonlinear variable
for the solid phase temperature. It should be named as follows:

!listing include/base/NS.h line=std::string T_solid

## Coupling with other Physics

In the presence of fluid flow, a [WCNSFVFluidHeatTransferPhysics.md] should be created
using the `[Physics/NavierStokes/FluidHeatTransfer/<name>]` syntax. The following input performs
the coupling between the fluid equations and the solid temperature equations. The coupling
between the fluid and solid domain is performed through a volumetric ambient convection term.

!listing test/tests/finite_volume/pwcns/channel-flow/2d-transient-physics.i block=Physics

!alert warning
Conjugate heat transfer on a surface on the boundary of the fluid domain is not currently implemented
with the `Physics` syntax. Please use a [FVConvectionCorrelationInterface.md] for that purpose.

!syntax parameters /Physics/NavierStokes/SolidHeatTransfer/PNSFVSolidHeatTransferPhysics

!syntax inputs /Physics/NavierStokes/SolidHeatTransfer/PNSFVSolidHeatTransferPhysics

!syntax children /Physics/NavierStokes/SolidHeatTransfer/PNSFVSolidHeatTransferPhysics
