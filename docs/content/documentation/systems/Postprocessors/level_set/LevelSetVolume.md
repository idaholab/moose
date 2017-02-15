# LevelSetVolume
A [Postprocessor](/Postprocessors/index.md) object utilized for computing the area or volume of the inside or
outside of a level set contour. The value computed from this postprocessor is an estimate computed using a
weighted average of the level set variables values at each quadrature point of an element.

## Example Syntax
!input modules/level_set/tests/reinitialization/master.i block=area indent=2 prefix=[Postprocessors] suffix=[] label=False

!parameters /UserObjects/LevelSetVolume

!inputfiles /UserObjects/LevelSetVolume

!childobjects /UserObjects/LevelSetVolume
