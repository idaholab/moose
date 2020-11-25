# ComputeElasticityTensorConstantRotationCP

!syntax description /Materials/ComputeElasticityTensorConstantRotationCP

## Description

The material `ComputeElasticityTensorConstantRotationCP` is used to create a constant
elasticity tensor for crystal plasticity simulations which refer only to the reference
configuration. As such, this class only rotates the elastic tensor once, during
the initial setup step.

This material builds an orthotropic elasticity tensor using the fill_method
`symmetric9` from [ComputeElasticityTensor](/ComputeElasticityTensor.md).


!alert warning title=Crystal Plasticity Simulations use Active Rotation
The rotation matrix used in this
class,`ComputeElasticityTensorConstantRotationCP`, is the transpose of the
rotation matrix created from the Bunge Euler angles in the base class,
[ComputeElasticityTensor](/ComputeElasticityTensor.md). This difference in the
rotation matrix is because of the active rotation convention used in crystal
plasticity simulations.

The fill method `symmetric9` is appropriate for materials with three orthotropic
planes of symmetry [!citep](malvern1969introduction), and is used for
simulations of anistropic materials such as cubic crystals.  The engineering
elasticity tensor notation for an orthotropic material is given in
[eq:symmetric9_fill_method]:
\begin{equation}
\label{eq:symmetric9_fill_method}
C_{ijkl} = \begin{bmatrix}
              C_{11} & C_{12} & C_{13} &      0 &      0 &      0 \\
              C_{12} & C_{22} & C_{23} &      0 &      0 &      0 \\
              C_{13} & C_{23} & C_{33} &      0 &      0 &      0 \\
                   0 &      0 &      0 & C_{44} &      0 &      0 \\
                   0 &      0 &      0 &      0 & C_{55} &      0 \\
                   0 &      0 &      0 &      0 &      0 & C_{66}
              \end{bmatrix}
\end{equation}

## Rotation Tensor Conventions

The [Euler angle convention](http://mathworld.wolfram.com/EulerAngles.html) used
in `ComputeElasticityTensorConstantRotationCP` is the $z$-$x'$-$z'$ (3-1-3)
convention.  The Euler angles arguments are expected in degrees, not radians,
and are denoted as $\phi_1$, $\Phi$, and $\phi_2$, corresponding to the axis
rotations.  The rotation tensor, $R$, is calculated from the current Euler
angles at each timestep as shown in [eq:rotation_tensor].
\begin{equation}
\label{eq:rotation_tensor}
  \begin{aligned}
  R_{11} & = cos(\phi_1)cos(\phi_2) - cos(\Phi)sin(\phi_1)sin(\phi_2) \\
  R_{12} & = -cos(\phi_1)sin(\phi_2) - cos(\Phi)cos(\phi_2)sin(\phi_1) \\
  R_{13} & = sin(\phi_1)sin(\Phi) \\
  R_{21} & = cos(\phi_2)sin(\phi_1) + cos(\phi_1)cos(\Phi)sin(\phi_2) \\
  R_{22} & = -sin(\phi_1)sin(\phi_2) + cos(\phi_1)cos(\Phi)cos(\phi_2) \\
  R_{23} & = -cos(\phi_1)sin(\Phi) \\
  R_{31} & = sin(\Phi) sin(\phi_2) \\
  R_{32} & = cos(\phi_2) sin(\Phi) \\
  R_{33} & = cos(\Phi)
  \end{aligned}
\end{equation}
The elasticity tensor is then rotated with [eq:cp_elasticity_tensor_rotation]
\begin{equation}
\label{eq:cp_elasticity_tensor_rotation}
  C'_{ijkl} = R^T_{im} R^T_{jn} R^T_{ko} R^T_{lp} C_{mnop}
\end{equation}
at the beginning of the simulation. This calculation is only performed once.

The crystal plasticity materials, including
`ComputeElasticityTensorConstantRotationCP` employ an active rotation: the
crystal system is rotated into the sample (loading) coordinate system. Generally
the Bunge Euler angles are used to describe a passive rotation: rotating the
sample coordinate system into the crystal coordinate system. As a result, the
rotation tensor applied is the transpose of the rotation tensor given in
[eq:rotation_tensor] as noted in [eq:cp_elasticity_tensor_rotation].

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/elasticity_tensor

!syntax parameters /Materials/ComputeElasticityTensorConstantRotationCP

!syntax inputs /Materials/ComputeElasticityTensorConstantRotationCP

!syntax children /Materials/ComputeElasticityTensorConstantRotationCP

!bibtex bibliography
