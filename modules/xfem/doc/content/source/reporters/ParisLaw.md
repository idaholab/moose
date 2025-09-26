# ParisLaw

## Description

The ParisLaw Postprocessor computes the crack extension size at all active crack front points in a fatigue crack growth scenario. Crack front points are provided by the [CrackMeshCut3DUserObject.md] UserObject and fracture integral values are provided by the [InteractionIntegral.md] vector postprocessor set-up by the [DomainIntegralAction.md].  The code is based on the Paris law and requires Paris law parameters [!param](/Reporters/ParisLaw/paris_law_c) and [!param](/Reporters/ParisLaw/paris_law_m) as the input. The amounts of crack extension at the crack front nodes are scaled such that the point with the largest effective K will have an extension equal to [!param](/Reporters/ParisLaw/max_growth_size).

## Example Syntax

!listing /modules/xfem/test/tests/solid_mechanics_basic/edge_crack_3d_fatigue.i block=Reporters

!syntax parameters /Reporters/ParisLaw

!syntax inputs /Reporters/ParisLaw

!syntax children /Reporters/ParisLaw
