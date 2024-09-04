# CappedWeakPlaneCosseratStressUpdate

!syntax description /Materials/CappedWeakPlaneCosseratStressUpdate

See [CappedWeakPlaneStressUpdate.md] for details on the non-Cosserat
capped weak plane stress model.

The difference between the Cosserat and non-Cosserat case is that the
stress tensor is potentially non-symmetric, and that only
$\sigma_{xz}$ and $\sigma_{yz}$ enter into the definition of $q$.
That is the equations in [CappedWeakPlaneStressUpdate.md#theory] for the stress variables hold as it is written.  This means
that the return-map process often results in a non-symmetric stress
tensor, even if the trial stress was symmetric.  The equations of
moment equilibrium then typically generate a spatially-varying moment
stress, the precise nature of which depends on the problem at hand,
such as the boundary conditions.

The differences between the presentation in [CappedWeakPlaneStressUpdate.md] for the non-Cosserat
case, and the Cosserat case are only:

- The equations for the ZX and ZY components of the stress (in [CappedWeakPlaneStressUpdate.md#rmap]) do not hold.  These components of stress take their trial values after the
  return-map problem has been solved: $\sigma_{zx} =
  \sigma_{zx}^{\mathrm{trial}}$ and $\sigma_{zy} =
  \sigma_{zy}^{\mathrm{trial}}$.
- The elasticity tensor need not have the symmetries given in
  the equations in section [CappedWeakPlaneStressUpdate.md#assumptions]: it only needs to satisfy
  $E_{ijkl}=E_{klij}$.


See the theory manual  (at [solid_mechanics/doc/theory/capped_weak_plane.pdf](https://github.com/idaholab/moose/tree/next/modules/solid_mechanics/doc/theory/capped_weak_plane.pdf))
for more explanation.

!syntax parameters /Materials/CappedWeakPlaneCosseratStressUpdate

!syntax inputs /Materials/CappedWeakPlaneCosseratStressUpdate

!syntax children /Materials/CappedWeakPlaneCosseratStressUpdate
