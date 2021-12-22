# Correspondence Plane Finite Strain Material

## Description

The `ComputePlaneFiniteStrainNOSPD` Material calculates the strain and rotation increments used in peridynamic correspondence models under finite strain assumptions for two dimensional analysis. The class first constructs the planar [deformation gradient](peridynamics/DeformationGradients.md) and uses the Rashid scheme [!citep](rashid1993incremental) to formulate a incremental corotational finite strain model.

Note that Material `ComputePlaneFiniteStrainNOSPD` must be used for _weak_ plane stress and generalized plane strain models to incorporate the out-of-plane strain components.

!syntax parameters /Materials/ComputePlaneFiniteStrainNOSPD

!syntax inputs /Materials/ComputePlaneFiniteStrainNOSPD

!syntax children /Materials/ComputePlaneFiniteStrainNOSPD
