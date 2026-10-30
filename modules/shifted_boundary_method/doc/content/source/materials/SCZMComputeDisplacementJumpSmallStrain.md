# SCZMComputeDisplacementJumpSmallStrain

!syntax description /Materials/SCZMComputeDisplacementJumpSmallStrain

The `SCZMComputeDisplacementJumpSmallStrain` material extrapolates displacement values from a
surrogate interface to the true interface using the displacement gradients and signed distance. It
then rotates the resulting jump into the local coordinate system defined by the true-interface
normal. This object is normally created by the
[ShiftedCohesiveZone action](ShiftedCohesiveZoneAction.md).

!syntax parameters /Materials/SCZMComputeDisplacementJumpSmallStrain

!syntax inputs /Materials/SCZMComputeDisplacementJumpSmallStrain

!syntax children /Materials/SCZMComputeDisplacementJumpSmallStrain
