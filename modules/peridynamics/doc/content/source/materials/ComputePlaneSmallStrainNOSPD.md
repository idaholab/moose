# Correspondence Plane Small Strain Material

## Description

Material `ComputePlaneSmallStrainNOSPD` calculates the [bond-associated deformation gradient](peridynamics/DeformationGradients.md) used in self-stabilized non-ordinary state-based peridynamic mechanics model under small strain assumptions for two dimensional analysis.

Note that Material `ComputePlaneFiniteStrainNOSPD` must be used for _weak_ plane stress and generalized plane strain models to incorporate the out-of-plane strain components.

!syntax parameters /Materials/ComputePlaneSmallStrainNOSPD

!syntax inputs /Materials/ComputePlaneSmallStrainNOSPD

!syntax children /Materials/ComputePlaneSmallStrainNOSPD
