# CappedWeakInclinedPlaneStressUpdate

!syntax description /Materials/CappedWeakInclinedPlaneStressUpdate

See [CappedWeakPlaneStressUpdate.md] for details on the
capped weak plane stress model with a non-inclined weak plane, where
the weak plane's normal is $(0, 0, 1)$.

This material accepts an arbitrary weak plane normal vector. In the reference frame where
the inclined plane's normal aligns with the $z$ axis, the assumptions
listed in [this section](CappedWeakPlaneStressUpdate.md#assumptions) must hold.

See the theory manual  (at [solid_mechanics/doc/theory/capped_weak_plane.pdf](https://github.com/idaholab/moose/tree/next/modules/solid_mechanics/doc/theory/capped_weak_plane.pdf))
for more explanation.

!syntax parameters /Materials/CappedWeakInclinedPlaneStressUpdate

!syntax inputs /Materials/CappedWeakInclinedPlaneStressUpdate

!syntax children /Materials/CappedWeakInclinedPlaneStressUpdate

