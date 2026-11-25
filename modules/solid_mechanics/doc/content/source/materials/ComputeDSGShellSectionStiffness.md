# ComputeDSGShellSectionStiffness

!syntax description /Materials/ComputeDSGShellSectionStiffness

## Description

This material computes the linear elastic section stiffness of a DSG shell element, the matrix that relates the nine generalized strains of [ComputeDSGShellStrain](/ComputeDSGShellStrain.md) to the nine stress resultants of [ComputeDSGShellResultants](/ComputeDSGShellResultants.md), from the Young's modulus, the Poisson's ratio and the thickness of a homogeneous isotropic plate. The matrix is block diagonal, see [DSGShellElements](/DSGShellElements.md):

- a membrane block $E t \mathbf{P}$, with $\mathbf{P}$ the plane stress matrix,
- a bending block $E t^3 / 12 \, \mathbf{P}$,
- a transverse shear block $\kappa G t \mathbf{I}$, with the shear correction factor $\kappa$ (5/6 by default),
- the drilling stiffness $G t$.

On the three-node element the transverse shear block carries the thin-limit factor $t^2 / (t^2 + \alpha h^2)$ of [!cite](lyly1993stable), with $h$ the longest edge of the element and $\alpha$ the parameter `thin_limit_factor` (0.15 by default). The factor keeps that element accurate for thin plates; setting the parameter to zero removes it.

The material also stores the thickness as the material property `dsg_shell_thickness`, read by [InertialForceDSGShell](/InertialForceDSGShell.md) and [DSGShellMassEigenKernel](/DSGShellMassEigenKernel.md).

### Choosing the parameters

- `youngs_modulus`, `poissons_ratio` and `thickness` describe the homogeneous isotropic plate; the resultants come out in the force unit of the modulus per unit length.
- `shear_correction_factor` scales the transverse shear stiffness $G t$ to account for the parabolic distribution of the transverse shear stress through the thickness. The default $5/6$ is the value of a homogeneous section and is the one to keep.
- `drilling_stiffness_factor` scales the drilling stiffness $G t$, the penalty that ties the drilling rotation to the rotation of the in-plane displacement field. The default 1 is the value of the formulation; a smaller value, 0.1 say, softens the penalty on membrane-dominated problems with coarse or distorted meshes, where the drilling constraint stiffens the response.
- `thin_limit_factor` is the constant $\alpha$ of the factor $t^2 / (t^2 + \alpha h^2)$ on the transverse shear stiffness of the three-node element, with $h$ its longest edge. The default 0.15 is the value of [!cite](lyly1993stable); it keeps that element from locking on meshes coarse relative to the thickness, has no effect on the other members, and zero turns it off.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/static/cantilever_strip.i block=Materials

!syntax parameters /Materials/ComputeDSGShellSectionStiffness

!syntax inputs /Materials/ComputeDSGShellSectionStiffness

!syntax children /Materials/ComputeDSGShellSectionStiffness

!bibtex bibliography
