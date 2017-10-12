# ComputeEigenstrainFromInitialStress
!syntax description /Materials/ComputeEigenstrainFromInitialStress

## Description

Computes an eigenstrain, $\epsilon$, defined by $\sigma_{\mathrm{initial}}=E\epsilon$, where $E$ is the elasticity tensor and $\sigma_{\mathrm{initial}}$ is the initial stress entered by the user.  The initial stress is entered as a vector of 9 Functions.

This allows a user to enter an insitu stress in rock-mechanics problems, for instance, by specifying the $zz$ component to be a function of depth into the ground, and the $xx$, $xy$, $yx$ and $yy$$ components of the initial stress to be related to the maximum and minimum principal horizontal tectonic stresses.  A general anisotropic initial stress is allowed.

The eigenstrain thus computed is added to the mechanical strain on the first time step, and is unchanged in subsequent timesteps.

!syntax parameters /Materials/ComputeEigenstrainFromInitialStress

!syntax inputs /Materials/ComputeEigenstrainFromInitialStress

!syntax children /Materials/ComputeEigenstrainFromInitialStress
