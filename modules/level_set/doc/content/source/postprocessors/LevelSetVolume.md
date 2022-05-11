# LevelSetVolume

A [Postprocessor](/Postprocessors/index.md) object utilized for computing the area or volume of the inside or
outside of a level set contour. The value computed from this postprocessor is an estimate computed using a
weighted average of the level set variables values at each quadrature point of an element.

## Example Syntax

!listing modules/level_set/test/tests/reinitialization/parent.i block=Postprocessors remove=cfl

!syntax parameters /Postprocessors/LevelSetVolume

!syntax inputs /Postprocessors/LevelSetVolume

!syntax children /Postprocessors/LevelSetVolume
