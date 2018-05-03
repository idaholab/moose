# Correspondence Finite Strain Material

## Description

Material `FiniteStrainNOSPD` calculates the strain and rotation increments used in self-stabilized non-ordinary state-based peridynamic mechanics model under finite strain assumptions. The class first constructs the bond-associated [deformation gradient](peridynamics/DeformationGradients.md) and uses the Rashid scheme [citet:rashid1993incremental] to formulate a incremental corotational finite strain model.

Note that Material `FiniteStrainNOSPD` can be used in general 3D and plane strain modeling and simulation.

!syntax parameters /Materials/FiniteStrainNOSPD

!syntax inputs /Materials/FiniteStrainNOSPD

!syntax children /Materials/FiniteStrainNOSPD

!bibtex bibliography
