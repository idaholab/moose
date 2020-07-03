# Displacement About Axis

!syntax description /BCs/DisplacementAboutAxis

## Description

The boundary condition class `DisplacementAboutAxis` applies a rotating displacement to the specified
mesh surface according to the user defined rotation function.  The boundary condition is always
applied to the displaced mesh.  The rotation function can be given in either radians or in angles,
and an axis of rotation can be specified with the `axis_origin` and `axis_direction` parameters. By default,
the function defines the rotation, but if the `angular_velocity` parameter is set, it allows for
the function to define an angular velocity instead of a total rotationa angle.

The rotating displacement value at the current node is calculated according to
[eq:rotating_displacement]:
\begin{equation}
\label{eq:rotating_displacement}
u_{rotation} = \boldsymbol{T}^{-1} \cdot \boldsymbol{R}_x^{-1} \cdot \boldsymbol{R}_y^{-1} \cdot \boldsymbol{R}_z \cdot \boldsymbol{R}_y \cdot \boldsymbol{R}_x \cdot \boldsymbol{T}
\end{equation}
where $\boldsymbol{T}$ is the translation matrix for axes of rotation not centered at the coordinate
system origin, and $\boldsymbol{R}_x$, $\boldsymbol{R}_y$, and $\boldsymbol{R}_z$ are rotation matrices about the
$\hat{x}$, $\hat{y}$, and $\hat{z}$ coordinate system axes, respectively.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction_cylinder.i
         block=BCs/top_x

A Function is required to prescribe either the total rotation or angular velocity of the `DisplacementAboutAxis` boundary condition
applied to the mesh. Either the name of another function defined elsewhere or a parsed
function is used as the argument for the `function` input parameter in the `DisplacementAboutAxis`
block. In this example, a parsed function is used.

!syntax parameters /BCs/DisplacementAboutAxis

!syntax inputs /BCs/DisplacementAboutAxis

!syntax children /BCs/DisplacementAboutAxis
