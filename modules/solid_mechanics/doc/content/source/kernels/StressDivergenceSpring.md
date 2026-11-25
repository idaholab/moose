# StressDivergenceSpring

!syntax description /Kernels/StressDivergenceSpring

## Description

This kernel applies the force of a spring law to one of the six degrees of freedom of a two-node spring element. It rotates the force and the tangent stiffness of the law from the local frame of [ComputeSpringDeformation](/ComputeSpringDeformation.md) to the global axes and writes the residual of the two nodes directly, as the truss kernel does: node 1 is pulled toward node 0 with the force, node 0 pushed the other way. The Jacobian has the rotated tangent on the diagonal block and the coupling among the three translations (or the three rotations) on the off-diagonal blocks, including the non-symmetric terms of the Mohr-Coulomb law, see [SpringElements](/SpringElements.md).

### Choosing the parameters

- `component` is the degree of freedom: 0, 1, 2 for the displacements along the global X, Y, Z axes and 3, 4, 5 for the rotations about them, and `variable` must be the matching entry of `displacements` or `rotations`; six kernels make a spring with six degrees of freedom, three a translational spring.
- `displacements` and `rotations` are the same variables given to the deformation material, the displacements for every component and the rotations for the components 3 to 5.
- `base_name` is the one of the deformation material and the law of this spring, when several springs share a block.
- `save_in` holds the residual of the kernel at the nodes, the global force of the spring at node 1 and its opposite at node 0, a second way to read the spring force beside the auxiliary kernels on the law's force property.

!listing modules/solid_mechanics/test/tests/spring/static/linear_spring.i block=Kernels

!syntax parameters /Kernels/StressDivergenceSpring

!syntax inputs /Kernels/StressDivergenceSpring

!syntax children /Kernels/StressDivergenceSpring

!bibtex bibliography
