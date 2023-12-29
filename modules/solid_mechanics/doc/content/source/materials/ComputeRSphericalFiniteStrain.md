# Compute R-Spherical Finite Strain

!syntax description /Materials/ComputeRSphericalFiniteStrain

## Description

The material `ComputeRSphericalFiniteStrain` calculates the small incremental strain for 1D
R-Spherical systems.

The 1D RSpherical materials and kernel are designed to model sphere geometries with 1D models.
Symmetry in the polar ($\theta$) and azimuthal ($\phi$) directions is assumed, and the model is
considered to revolve in both of these directions.  In the 1D R-Spherical code, the material
properties, variables (e.g. temperature), and loading conditions are all assumed to be spherically
symmetric: these attributes only depend on the axial position.

!alert note
The `COORD_TYPE` in the Problem block of the input file must be set to RSPHERICAL.

As in the plane strain and axisymmetric cases, the stress and strain tensors are modified in the
spherical problem; only the diagonal components are non-zero in this 1D problem.
\begin{equation}
\begin{bmatrix}
\epsilon_{rr} & 0 & 0 \\
0 & \epsilon_{\theta \theta} & 0 \\
0 & 0 & \epsilon_{\phi \phi}
\end{bmatrix}
\end{equation}
where the value of the normal strain components in the polar and azimuth directions $\epsilon_{\theta
\theta}$ and $\epsilon_{\phi \phi}$ depends on the displacement and position in the radial direction
\begin{equation}
  \label{eq:polar_azimuthal_rspherical_strains}
  \epsilon_{\theta \theta} = \epsilon_{\phi \phi} = \frac{u_r}{X_r}.
\end{equation}
Although axisymmetric problems solve for 3D stress and strain fields, the problem is mathematically
1D.  In the cylindrical coordinate axisymmetric system, the values of stress and strain in the
$\theta$ and $\phi$ directions do not depend on the $\theta$ or $\phi$ coordinates.

Once the deformation gradient is calculated for the 1D geometry, the deformation gradient is passed
to the strain and rotation methods used by default 3D Cartesian simulations, as described in the
[Finite Strain Class](ComputeFiniteStrain.md) page.

## Example Input File Syntax

The finite incremental R-spherical strain calculator can be activated in the input file through the
use of the TensorMechanics Master Action, as shown below.

!listing modules/tensor_mechanics/test/tests/1D_spherical/finiteStrain_1DSphere_hollow.i
         block=Modules/TensorMechanics/Master

!alert note title=Use of the Tensor Mechanics Master Action Recommended
The [TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md) is designed to
automatically determine and set the strain and stress divergence parameters correctly for the
selected strain formulation.  We recommend that users employ the
[TensorMechanics Master Action](syntax/Modules/TensorMechanics/Master/index.md) whenever possible
to ensure consistency between the test function gradients and the strain formulation selected.

!syntax parameters /Materials/ComputeRSphericalFiniteStrain

!syntax inputs /Materials/ComputeRSphericalFiniteStrain

!syntax children /Materials/ComputeRSphericalFiniteStrain
