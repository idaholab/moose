# Rank Two Invariant

!syntax description /Materials/RankTwoInvariant

## Description

This is a Material model used to extract an invariant of a rank-2 tensor in a
Cartesian coordinate system. This can be used regardless of the coordinate
system used by the model.

This Material model is set up by
[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md) automatically
when stress components are requested in the generate_output parameter, but can
also be set up directly by the user. This class provides the ability to compute
VonMises, Effective, Hydrostatic, L2norm, Volumetric, Triaxiality, MaxShear,
StressIntensity, First, Second, and Third Invariant as well as Max, Mid, and Min
Primary stresses ($\boldsymbol{\sigma}$) and strains
($\boldsymbol{\epsilon}$)quantities for a Rank-2 tensor, as described in
[RankTwoScalarTools](RankTwoScalarTools.md).  


The component of the rank-2 tensor extracted is stored as a scalar material
property, which allows for it to be more accurately represented in calculations
that use this quantity at quadrature points than would be possible using the
related [RankTwoScalarAux](RankTwoScalarAux.md)

!syntax parameters /Materials/RankTwoInvariant

!syntax inputs /Materials/RankTwoInvariant

!syntax children /Materials/RankTwoInvariant
