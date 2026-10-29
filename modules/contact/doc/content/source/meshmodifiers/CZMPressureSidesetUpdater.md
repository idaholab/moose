# CZMPressureSidesetUpdater

!syntax description /MeshModifiers/CZMPressureSidesetUpdater

## Description

`CZMPressureSidesetUpdater` builds sidesets on the portions of a mortar cohesive zone interface
that have fully debonded, so that loads such as a gas pressure can be applied to the walls of the
void that opens between the two surfaces. As the cohesive zone damage evolves, every segment of the
interface whose damage has reached one is added to two sidesets: the face on the secondary side is
added to [!param](/MeshModifiers/CZMPressureSidesetUpdater/pressure_boundary_secondary), and every
face on the primary side that the secondary face was bonded to is added to
[!param](/MeshModifiers/CZMPressureSidesetUpdater/pressure_boundary_primary). Together the two
sidesets bound the debonded region, so they can be used both to apply boundary conditions to the
void and to measure it, for example with [InternalVolume](InternalVolume.md) .

A segment is considered fully separated when the damage given by
[!param](/MeshModifiers/CZMPressureSidesetUpdater/damage_variable) is at or above
[!param](/MeshModifiers/CZMPressureSidesetUpdater/threshold) at every quadrature point of a
secondary lower-dimensional element. The damage is normally provided by a
[CohesiveZoneMortarUserObjectAux](CohesiveZoneMortarUserObjectAux.md) with
`cohesive_zone_quantity = cohesive_damage`. The default threshold is $1 - 10^{-10}$, so that only
segments the cohesive zone model reports as fully broken are added; it is not intended as a tuning
parameter.

The primary faces paired with each secondary face are those that overlapped it in the undisplaced
configuration, determined once at the start of the simulation from a mortar projection between
[!param](/MeshModifiers/CZMPressureSidesetUpdater/secondary_boundary) and
[!param](/MeshModifiers/CZMPressureSidesetUpdater/primary_boundary). Pairing in the reference
configuration ensures that the faces added to the two sidesets remain the faces that were bonded
together, however far the surfaces have opened or slid relative to each other. When the primary and
secondary meshes do not conform, a secondary face may be paired with several primary faces, and all
of them are added.

Faces are only ever added, never removed, because a debonded cohesive zone does not heal. The mesh
is marked as changed only on time steps in which new faces are added, so that objects acting on the
two sidesets, such as `Pressure` boundary conditions, are reinitialized to include the new faces.
Because the sidesets are updated at the end of a time step, after the solve, a face that debonds
during a time step is loaded by such boundary conditions from the next time step onward.

## Example Input Syntax

!listing modules/combined/test/tests/czm_pressure_sidesets/czm_pressure_sideset.i block=MeshModifiers

!syntax parameters /MeshModifiers/CZMPressureSidesetUpdater

!syntax inputs /MeshModifiers/CZMPressureSidesetUpdater

!syntax children /MeshModifiers/CZMPressureSidesetUpdater
