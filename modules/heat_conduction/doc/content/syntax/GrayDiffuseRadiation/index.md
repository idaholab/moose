# The Net Radiation Method System

The `GrayDiffuseRadiation` syntax invokes the
[RadiationTransferAction](/RadiationTransferAction.md) action. It simplifies setting up
gray, diffuse radiative exchange problems. It requires the user to provide sideset IDs,
emissivities on these sidesets, and the type of sideset.It allows to split the sidesets
into radiation patches using the `n_patches` parameter.
Refer to [RadiationTransferAction](/RadiationTransferAction.md) for details.

!syntax list /GrayDiffuseRadiation objects=True actions=False subsystems=False

!syntax list /GrayDiffuseRadiation objects=False actions=False subsystems=True

!syntax list /GrayDiffuseRadiation objects=False actions=True subsystems=False

## Example Input syntax

!listing modules/heat_conduction/test/tests/radiation_transfer_action/radiative_transfer_action.i
block=GrayDiffuseRadiation/cavity
