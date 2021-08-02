# Hill Constants

!syntax description /Materials/HillConstants

## Description

`HillConstants` material transforms the user specified hill constants from global co-ordinates to the material co-ordinates and stores them as a material property. It rotates the hill constants during the initial time step only. This class does not rotate the hill tensor during the simulation.  The initial rotation is performed based of the user defined rotation angle parameters. The rotated Hill tensor is obtained as [!cite](stewart2011anisotropic):

\begin{equation}
\label{eq:rotate_hill_tensor}
  H_{rot} = T_m H T_m^T
\end{equation}
 Here, $H_{rot}$ is the rotated hill tensor, $H$ is the initial Hill tensor and $T_m$ is the transformation matrix.

The transformation matrix is formed assuming anti-clockwise rotation about z-, y-, and x-, axis in sequence [!cite](bower2009applied) such that
\begin{equation}
\label{eq:transformation_matrix}
 T_m = T_x T_y T_z
\end{equation}
Here, $T_x$, $T_y$, $T_z$ are the transformation matrices about x-, y- and z-axis, respectively:
\begin{equation}
\begin{aligned}
\label{eq:rotation_matrices}
 T_x &=   &{\begin{bmatrix}
           1 & 0 & 0 & 0 & 0& 0\\
           0 & \cos{\alpha}^2 & \sin{\alpha}^2& 2 \cos{\alpha} \sin{\alpha}& 0& 0 \\
           0& {\sin{\alpha}}^2& {\cos{\alpha}}^2& -2 \cos{\alpha} \sin{\alpha}& 0& 0 \\
           0& - \cos{\alpha} \sin{\alpha}& \cos{\alpha} \sin{\alpha}& {\cos{\alpha}}^2 - {\sin{\alpha}}^2 & 0& 0 \\
           0& 0& 0& 0& \cos{\alpha}& -\sin{\alpha}\\
           0& 0& 0& 0& \sin{\alpha}& \cos{\alpha}
           \end{bmatrix}} \\
 T_y &=   &{\begin{bmatrix}
            \cos{\beta}^2& 0& \sin{\beta}^2& 0& 2\cos{\beta} \sin{\beta}& 0\\
            0& 1& 0& 0&0& 0\\
            {\sin{\beta}}^2& 0& \cos{\beta}^2& 0& -2 \cos{\beta} \sin{\beta}& 0\\
            0& 0& 0& \cos{\beta}& 0& -\sin{\beta}\\
            -\cos{\beta} \sin{\beta}& 0&  \cos{\beta} \sin{\beta}& 0& \cos{\beta}^2 -\sin{\beta}^2& 0\\
            0& 0& 0& \sin{\beta}& 0& \cos{\beta}
            \end{bmatrix}} \\
  T_z &=  &{\begin{bmatrix}
             \cos{\theta}^2 & \sin{\theta}^2 & 0 & 0 & 0 & 2 \cos{\theta} \sin{\theta}\\
             \sin{\theta}^2 & \cos{\theta}^2 & 0 & 0 & 0 & - 2 \cos{\theta} \sin{\theta}\\
             0 & 0 & 1 & 0 & 0 & 0\\
             0 & 0 & 0 & \cos{\theta}& \sin{\theta} & 0\\
             0 & 0 & 0 & - \sin{\theta}& \cos{\theta} & 0\\
             -\cos{\theta} \sin{\theta} & \cos{\theta} \sin{\theta} & 0 & 0 & 0 & \cos{\theta}^2-\sin{\theta}^2
             \end{bmatrix}}
\end{aligned}
\end{equation}
where, $\alpha$, $\beta$ and $\theta$ are the rotation angles. Note that, to reduce the computational cost, only the components required for the updated hill constants are computed from the rotated Hill tensor.

## Temperature dependency and large deformation kinematics

The Hill coefficients vary depending on temperature (due to texture) and depending on the material's rotation. To that effect, the user can create functions describing the evolution of the coefficients with temperature and provide it to the material, as follows:

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_creep/ad_aniso_creep_temperature_coefficients_function_variation.i block=Materials/hill_constants

where six piecewise linear functions are created to define the material directional creep dependency with temperature.

In addition, or independently, the user can choose to account for finite strain rotation to update these coefficients. To do so, the user needs to set the input argument `use_large_rotation` to true, which will trigger the use of [eq:rotate_hill_tensor] to account for the rotation. Updating of directional coefficients with finite strain rotation kinematics is stronly encouraged when the material is expected to suffer large deformation (e.g. ballooning) or to undergo rigid body rotation (e.g. significant bending of a beam or pipe).

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_creep/3d_bar_orthotropic_90deg_rotation_ad_creep_z.i block=Materials/hill_tensor

!alert note
Temperature coupling and large deformation updates have only been tested on creep models.

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_plasticity/ad_aniso_plasticity_x.i block=Materials/hill_tensor

!syntax parameters /Materials/HillConstants

!syntax inputs /Materials/HillConstants

!syntax children /Materials/HillConstants

!bibtex bibliography
