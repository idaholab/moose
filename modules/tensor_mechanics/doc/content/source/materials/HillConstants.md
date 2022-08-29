# Hill Constants

!syntax description /Materials/HillConstants

## Description

The `HillConstants` material transforms the user specified hill constants from global co-ordinates to the material co-ordinates and stores them as a material property. It rotates the hill constants during the initial time step only. This class does not rotate the hill tensor during the simulation.  The initial rotation is performed based of the user defined rotation angle parameters. The rotated Hill tensor is obtained as [!cite](stewart2011anisotropic):

\begin{equation}
\label{eq:rotate_hill_tensor}
  H_{rot} = T_m H T_m^T
\end{equation}

Here, $H_{rot}$ is the transformed hill tensor, $H$ is the initial Hill tensor and $T_m$ is the transformation matrix. The transformation matrix $T_m$ is formed from the components of the total rotation matrix due to initial rotation and the orientation change due to large deformation.

## Temperature dependency and large deformation kinematics

The Hill's tensor varies depending on temperature (due to texture) and depending on the material's rotation. To that effect, the user can create functions describing the evolution of the coefficients with temperature and provide it to the material, as follows:

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_creep/ad_aniso_creep_temperature_coefficients_function_variation.i block=Materials/hill_constants

where six piecewise linear functions are created to define the material directional creep dependency with temperature.

In addition, the code accounts for finite strain rotation to update the Hill's tensor by default. For simple problems, the user can deactivate this feature by setting the input argument `use_large_rotation` to false. Updating of directional coefficients with finite strain rotation kinematics is stronly encouraged when the material Similarly, use of the transformed Hill's tensor would need to be disable in the creep model, i.e. the following input file's command line needs to be passed: `use_transformation = false`.

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_creep/3d_bar_orthotropic_90deg_rotation_ad_creep_z.i block=Materials/hill_tensor

!alert note
Temperature coupling and large deformation updates have only been tested on creep models.

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/anisotropic_plasticity/ad_aniso_plasticity_x.i block=Materials/hill_tensor

!syntax parameters /Materials/HillConstants

!syntax inputs /Materials/HillConstants

!syntax children /Materials/HillConstants

!bibtex bibliography
