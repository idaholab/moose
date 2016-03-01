# Correspondence Finite Strain Material

## Description

The `ComputeFiniteStrainNOSPD` Material calculates the strain and rotation increments used in the peridynamic correspondence models under finite strain assumptions. The class first constructs the [deformation gradient](peridynamics/DeformationGradients.md) and uses the Rashid scheme [citep:](rashid1993incremental) to formulate a incremental corotational finite strain model.

Note that Material `ComputeFiniteStrainNOSPD` can be used in general 3D and plane strain modeling and simulation.

!syntax parameters /Materials/ComputeFiniteStrainNOSPD

!syntax inputs /Materials/ComputeFiniteStrainNOSPD

!syntax children /Materials/ComputeFiniteStrainNOSPD
