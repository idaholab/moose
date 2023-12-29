# ComputeEigenstrainFromInitialStress

!syntax description /Materials/ComputeEigenstrainFromInitialStress

## Description

Computes an eigenstrain, $\epsilon$, defined by $\sigma_{\mathrm{initial}}=E\epsilon$, where $E$ is
the elasticity tensor and $\sigma_{\mathrm{initial}}$ is the initial stress entered by the user.  The
initial stress is entered as a vector of 9 Functions, which may be optionally multiplied by a vector of 9 AuxVariables.

This allows a user to enter an insitu stress in rock-mechanics problems, for instance, by specifying
the $zz$ component to be a function of depth into the ground, and the $xx$, $xy$, $yx$ and $yy$
components of the initial stress to be related to the maximum and minimum principal horizontal
tectonic stresses.  A general anisotropic initial stress is allowed.  Using AuxVariables to set the initial stress may be advantageous when reading from a solution file using a [SolutionAux](/SolutionAux.md).

The eigenstrain thus computed is added to the mechanical strain on the first time step, and is
unchanged in subsequent timesteps.  The eigenstrain thus computed is given a name that is specified
by the user, and that name must be included in the `eigenstrain_names` input parameter of the strain
calculator (eg, in `ComputeSmallStrain`), otherwise MOOSE will not add the eigenstrain to the
mechanical strain!

!alert warning title=Time-dependent elasticity tensors
The eigenstrain is computed on the first time step, using the elasticity tensor; hence, the
elasticity tensor should not be time-varying in the first timestep.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/initial_stress/gravity.i block=Materials

In this example the eigenstrain is given the name `ini_stress`.  This name is passed to the
`ComputeSmallStrain` strain calculator using the `eigenstrain_names` parameter.  The initial stress
is defined by the functions defined in

!listing modules/tensor_mechanics/test/tests/initial_stress/gravity.i block=Functions

!syntax parameters /Materials/ComputeEigenstrainFromInitialStress

!syntax inputs /Materials/ComputeEigenstrainFromInitialStress

!syntax children /Materials/ComputeEigenstrainFromInitialStress
