# LinearFVInletOutletScalarBC

!syntax description /LinearFVBCs/LinearFVInletOutletScalarBC

## Description

`LinearFVInletOutletScalarBC` is a boundary condition for an advected scalar at a boundary
through which the flow may leave or re-enter. It switches on the sign of the outward value of
[!param](/LinearFVBCs/LinearFVInletOutletScalarBC/face_flux) on each face:

- On outflow, it is a zero-gradient outlet, the same as
  [LinearFVAdvectionDiffusionOutflowBC](LinearFVAdvectionDiffusionOutflowBC.md): the face value is
  extrapolated from the cell, with a two-term expansion when
  [!param](/LinearFVBCs/LinearFVInletOutletScalarBC/use_two_term_expansion) is true, and the
  normal gradient is zero.
- On backflow, it imposes [!param](/LinearFVBCs/LinearFVInletOutletScalarBC/backflow_value) as
  the face value, and the normal gradient is the difference between that value and the cell value
  over the cell-to-face distance.

This boundary condition is created by
[WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics](WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics.md)
for the volume fraction on every outlet boundary of the coupled flow. The physics sets
[!param](/LinearFVBCs/LinearFVInletOutletScalarBC/backflow_value) from
`volume_fraction_outlet_backflow_functor`, which defaults to zero so that only gas enters through
an outlet, and [!param](/LinearFVBCs/LinearFVInletOutletScalarBC/use_two_term_expansion) to
false: the outflow extrapolation that `volume_fraction_two_term_bc_expansion` requests is applied
by [ConservativeDiffuseInterfaceVOFMULESCorrector](ConservativeDiffuseInterfaceVOFMULESCorrector.md)
as a limited correction. It reads the backflow switch from
`lagged_corrected_face_phi`, the face flux frozen for the current outer iteration, which is also
the flux the volume fraction is advanced with.

## Example Input Syntax

In this example, the top boundary of a dam break is an outlet of the flow. The volume-fraction
physics creates this boundary condition on it, so the volume fraction is extrapolated where air
leaves through the top and set to zero, pure air, where air is drawn back in.

!listing modules/navier_stokes/examples/dam_break/dam_break.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated

!syntax parameters /LinearFVBCs/LinearFVInletOutletScalarBC

!syntax inputs /LinearFVBCs/LinearFVInletOutletScalarBC

!syntax children /LinearFVBCs/LinearFVInletOutletScalarBC
