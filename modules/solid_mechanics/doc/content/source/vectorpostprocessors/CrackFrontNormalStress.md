# CrackFrontNormalStress

!syntax description /VectorPostprocessors/CrackFrontNormalStress

## Description

This object computes the average stress normal to the crack defined by the [CrackFrontDefinition.md].  Data produced by this vectorPostProcessor is used in conjunction with the [InteractionIntegral.md] in the XFEM module by the `MeshCut2DFractureUserObject` to grow cracks. The `CrackFrontNormalStress` is useful for extending cracks near free surfaces where the interaction integrals used to compute `KI` and `KII` are reduced due to the integration domain intersecting a free surface.

## Theory

Details on the theory behind the computation of the various fracture integrals, including the $J$-Integral, are provided [here](FractureIntegrals.md).

!syntax parameters /VectorPostprocessors/CrackFrontNormalStress

!syntax inputs /VectorPostprocessors/CrackFrontNormalStress

!syntax children /VectorPostprocessors/CrackFrontNormalStress
