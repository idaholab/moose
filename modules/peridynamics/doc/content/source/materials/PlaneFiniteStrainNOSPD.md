# Correspondence Plane Finite Strain Material

## Description

Material `PlaneFiniteStrainNOSPD` calculates the strain and rotation increments used in self-stabilized non-ordinary state-based peridynamic mechanics model under finite strain assumptions for two dimensional analysis. The class first constructs the planar bond-associated [deformation gradient](peridynamics/DeformationGradients.md) and uses the Rashid scheme [citet:rashid1993incremental] to formulate a incremental corotational finite strain model.

Note that Material `PlaneFiniteStrainNOSPD` must be used for _weak_ plane stress and generalized plane strain models to incorporate the out-of-plane strain components.

!syntax parameters /Materials/PlaneFiniteStrainNOSPD

!syntax inputs /Materials/PlaneFiniteStrainNOSPD

!syntax children /Materials/PlaneFiniteStrainNOSPD

!bibtex bibliography
