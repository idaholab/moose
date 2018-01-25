# Displacement About Axis
!syntax description /BCs/DisplacementAboutAxis

## Description
The boundary condition class `DisplacementAboutAxis` applies a rotating displacement to the specified mesh surface according to the user defined rotation function.
The boundary condition is always applied to the displaced mesh.
The rotation function can be given in either radians or in angles, and an axis of rotation can be specified with the `axis_origin` and `axis_direction` parameters.

The rotating displacement value at the current node is calculated according to \eqref{eq:rotating_displacement}:
\begin{equation}
\label{eq:rotating_displacement}
u_{rotation} = \mathbf{T}^{-1} \cdot \mathbf{R}_x^{-1} \cdot \mathbf{R}_y^{-1} \cdot \mathbf{R}_z \cdot \mathbf{R}_y \cdot \mathbf{R}_x \cdot \mathbf{T}
\end{equation}
where $\mathbf{T}$ is the translation matrix for axes of rotation not centered at the coordinate system origin, and $\mathbf{R}_x$, $\mathbf{R}_y$, and $\mathbf{R}_z$ are rotation matrices about the $\hat{x}$, $\hat{y}$, and $\hat{z}$ coordinate system axes, respectively.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction_cylinder.i block=BCs/top_x

A Function is required to prescribe the rate of the `DisplacementAboutAxis` boundary condition application to the mesh. Note that the name of the Function is used as the argument for the `function` input parameter in the `DisplacementAboutAxis` block.
!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction_cylinder.i block=Functions/rampConstantAngle

!syntax parameters /BCs/DisplacementAboutAxis

!syntax inputs /BCs/DisplacementAboutAxis

!syntax children /BCs/DisplacementAboutAxis
