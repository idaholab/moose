# LinearSpring

!syntax description /Materials/LinearSpring

## Description

Linear spring law: the force along each local axis of the spring is a constant stiffness times the deformation along that axis, see [SpringElements](/SpringElements.md). In the rotational mode the moment about each axis is the stiffness times the relative rotation.

### Choosing the parameters

- `stiffness` holds the three constants in the order of the local axes of [ComputeSpringDeformation](/ComputeSpringDeformation.md); a zero leaves that axis free. Dimension force per length (moment per radian in the rotational mode); when `tributary_area` is given the stiffness is per unit area, force per length cubed, a subgrade modulus.
- `mode` selects the translations (default) or the rotations; a spring with six degrees of freedom takes two LinearSpring materials, one of each mode, on the same block.
- `tributary_area` multiplies every force; it is the area of the interface the spring represents, so that the same stiffness per unit area serves a bed of springs of different spacings.
- `base_name` as on [ComputeSpringDeformation](/ComputeSpringDeformation.md), for several springs on one block.

!listing modules/solid_mechanics/test/tests/spring/static/six_dof.i block=Materials

!media media/solid_mechanics/spring_law_linear.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_linear_law
      caption=The linear law.

!syntax parameters /Materials/LinearSpring

!syntax inputs /Materials/LinearSpring

!syntax children /Materials/LinearSpring

!bibtex bibliography
