# AbaqusTotalUELEnergy

!syntax description /Postprocessors/AbaqusTotalUELEnergy

## Description

`AbaqusTotalUELEnergy` sums one component of the Abaqus `ENERGY` array over all elements. A UEL
plugin fills the 8-entry `ENERGY` array for each element. The
[!param](/Postprocessors/AbaqusTotalUELEnergy/component) parameter selects the zero-based entry to
sum.

The postprocessor reads the energy from the
[AbaqusUserElement](AbaqusUserElement.md) objects named in
[!param](/Postprocessors/AbaqusTotalUELEnergy/uel_user_object). Each of these objects must set
`use_energy = true`, or the postprocessor stops with an error.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/uel/uel_energy.i block=Postprocessors

!syntax parameters /Postprocessors/AbaqusTotalUELEnergy

!syntax inputs /Postprocessors/AbaqusTotalUELEnergy

!syntax children /Postprocessors/AbaqusTotalUELEnergy
