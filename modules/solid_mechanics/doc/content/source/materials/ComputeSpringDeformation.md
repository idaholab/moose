# ComputeSpringDeformation

!syntax description /Materials/ComputeSpringDeformation

## Description

This material builds the local frame of a two-node spring element and reports the translation and the rotation of node 1 relative to node 0 in that frame, tension positive along axis 1, as `spring_deformation` and `spring_rotation` (both `RealVectorValue`), with the rotation matrix `spring_rotation_matrix` whose rows are the local axes. The spring laws ([LinearSpring](/LinearSpring.md) and the others of [SpringElements](/SpringElements.md)) turn these into forces, and [StressDivergenceSpring](/StressDivergenceSpring.md) applies them. The values are constant over the element and stored at every quadrature point.

### Choosing the parameters

- `displacements` and `rotations` are the variables of the structure the spring is part of, three of each: the family is written for three-dimensional models, and a plane model holds its third displacement. The rotations are needed only for a rotational law.
- `local_axis_1` and `local_axis_2` fix the frame when the element direction is not the direction wanted: a short element standing for a spring between coincident positions (see [SpringElements](/SpringElements.md#zero-length)), or the normal of a wall for a Mohr-Coulomb interface. Without them axis 1 runs along the element and axis 2 is the projection of the global Y axis (the global axis least aligned with axis 1 when the element is along Y), which puts the local axes of a spring along X on X, Y and Z.
- `compute_rates` is set for the [ViscousSpring](/ViscousSpring.md) law and needs a transient executioner, since the rates come from the time integrator.
- `base_name` prefixes the property names when several springs share a block; the laws and the kernels of that spring take the same one.

!listing modules/solid_mechanics/test/tests/spring/static/linear_spring.i block=Materials

!syntax parameters /Materials/ComputeSpringDeformation

!syntax inputs /Materials/ComputeSpringDeformation

!syntax children /Materials/ComputeSpringDeformation

!bibtex bibliography
