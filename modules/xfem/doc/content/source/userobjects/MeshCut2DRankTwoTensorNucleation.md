# MeshCut2DRankTwoTensorNucleation

!syntax description /UserObjects/MeshCut2DRankTwoTensorNucleation

## Overview

The XFEM nucleation UserObject `MeshCut2DRankTwoTensorNucleation` defines new cracks that are added to the cutter
mesh used by [MeshCut2DFractureUserObject.md] when a user-defined nucleation criterion is met.
The nucleation criterion is based on the [!param](/UserObjects/MeshCut2DRankTwoTensorNucleation/scalar_type) extracted from a `RankTwoTensor` specified by [!param](/UserObjects/MeshCut2DRankTwoTensorNucleation/tensor) (used to store stresses and strains in TensorMechanics),
such as a principal stress or a component of stress.

If the scalar exceeds a threshold specified by [!param](/UserObjects/MeshCut2DRankTwoTensorNucleation/nucleation_threshold), a line segment with a length specified by [!param](/UserObjects/MeshCut2DRankTwoTensorNucleation/nucleation_length) will be inserted
into the [MeshCut2DFractureUserObject.md] cutter mesh.
Cracks are only allowed to initiate from elements on boundaries specified by [!param](/UserObjects/MeshCut2DRankTwoTensorNucleation/initiate_on_boundary).
Once the nucleation criterion is reached, a line segment of the specified length is inserted into the cutter mesh,
centered on the element centroid it nucleates from.
The direction of the nucleated crack is normal to the direction returned by the `RankTwoTensor` scalar.
For example, `MaxInPlanePrincipal` returns the direction of the maximum in-plane principal component and the crack direction is normal to this.
The nucleation length should be at least the length of the element it nucleates in so that the nucleated crack will completely cut the element.
A crack will only be nucleated if it is at least a distance specified by [!param](/UserObjects/MeshCut2DRankTwoTensorNucleation/nucleation_radius) away from existing or nucleated cracks.
If multiple cracks nucleate in the same xfem update and are within the specified `nucleation_radius`, the crack nucleated from the element with the lowest id will be retained and no other cracks within the `nucleation_radius` will be nucleated.

`MeshCut2DRankTwoTensorNucleation` copies several features available in the [XFEMRankTwoTensorMarkerUserObject.md].
These include the nucleation threshold being provided as a coupled variable and the computation of the maximum value of the scalar
quantity used for nucleation.
By providing the nucleation threshold as a coupled variable, it can be specified as either a constant or variable value.
Coupled variable input is useful for introducing randomness in the strength by using an AuxVariable that has been initialized
with a random initial condition. For example, the [VolumeWeightedWeibull.md] initial condition is well-suited for this purpose.
The determination of crack nucleation is based on the average value of the scalar quantity over all the quadrature points in an element.

## Example Input File Syntax

!listing test/tests/nucleation_uo/nucleate_AllEdgeCracks.i block=UserObjects/nucleate

!syntax parameters /UserObjects/MeshCut2DRankTwoTensorNucleation

!syntax inputs /UserObjects/MeshCut2DRankTwoTensorNucleation

!syntax children /UserObjects/MeshCut2DRankTwoTensorNucleation
