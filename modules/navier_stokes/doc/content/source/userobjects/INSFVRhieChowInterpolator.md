# INSFVRhieChowInterpolator

!syntax description /UserObjects/INSFVRhieChowInterpolator

## Overview

This object coordinates everything about an incompressible Navier-Stokes finite
volume (INSFV) simulation related to Rhie-Chow. This object performs the
following activities

- Loops through all momentum residual objects and gathers information related to
- $a$ coefficients: these are the coefficients that multiply the
  on-diagonal/current-cell velocity solution in a linearized writing of the
  momentum equation

- Computes the Rhie-Chow velocity according to [rcvel] when requested by
  advection kernels or postprocessors

\begin{equation}
\label{rcvel}
\bm{v}_f = \overline{\bm{v}_f} - \overline{\bm{D}_f}\left(\nabla p_f - \overline{\nabla p_f}\right)
\end{equation}

### Accessing 'a' coefficient data in nonstandard locations

If you ever come across an error like

```
Attempted access into CellCenteredMapFunctor 'a' with a key that does not yet
exist in the map. Make sure to fill your CellCenteredMapFunctor for all elements
you will attempt to access later.
```

then it means an object you're using in your input file is attempting to access
Rhie-Chow data in unexpected locations. This can potentially be remedied by
setting the parameter `pull_all_nonlocal_a = true`. This will tell all processes
to pull 'a' coefficient data for all elements they have access to (which may not
be all the elements in the mesh if the mesh is distributed) from the processes
that own the 'a' coefficient data.

### Correcting for non-velocity-dependent volume forces

The standard Rhie-Chow interpolation may introduce oscillatory errors in the velocity
field under block-localized volume forces that do not depend on velocity, e.g., a pump force.

The user can activate the correction for body forces by setting `correct_volumetric_force = true`.
Then, the list of the names of the functors representing the body forces must be defined under
the `volumetric_force_functors` parameter.
Finally, the user can select two possible methods for correcting for volume force under the
`volume_force_correction_method` parameter:

-  `pressure-consistent`, which deactivates the Rhie-Chow correaction on faces in which the pressure
  correction is inconsistent due to the volume forces
-  `force-consistent`, which deactivates the Rhie-Chow correction in the regions with localized
  volume forces since the pressure-velocity system is already linked by the volume force in these regions.

The latter method is the default and is generally preferred due to its lower computational cost.

!syntax parameters /UserObjects/INSFVRhieChowInterpolator

!syntax inputs /UserObjects/INSFVRhieChowInterpolator

!syntax children /UserObjects/INSFVRhieChowInterpolator
