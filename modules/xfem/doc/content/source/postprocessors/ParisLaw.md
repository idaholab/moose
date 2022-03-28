# ParisLaw

## Description

The ParisLaw Postprocessor computes the crack extension size at all active crack front points in a fatigue crack growth scenario. The code is based on the Paris law and requires Paris law parameters as the input. The amounts of crack extension at the crack front nodes are scaled such that the point with the largest effective K will have an extension equal to the input maximum extension size.

## Example Syntax

!listing /modules/xfem/test/tests/solid_mechanics_basic/edge_crack_3d_fatigue.i block=Postprocessors

!syntax parameters /Postprocessors/ParisLaw

!syntax inputs /Postprocessors/ParisLaw

!syntax children /Postprocessors/ParisLaw
