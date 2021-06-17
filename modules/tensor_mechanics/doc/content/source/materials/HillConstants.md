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
Here, T_x, T_y, T_z are the transformation matrices about x-, y- and z-axis, respectively:
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

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_plasticity/ad_aniso_plasticity_x.i
         block=Materials/hill_tensor

!syntax parameters /Materials/HillConstants

!syntax inputs /Materials/HillConstants

!syntax children /Materials/HillConstants

!bibtex bibliography
