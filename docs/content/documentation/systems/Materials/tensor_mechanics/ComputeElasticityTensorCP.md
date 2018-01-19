# Compute Elasticity Tensor CP
!syntax description /Materials/ComputeElasticityTensorCP

## Description
The material `ComputeElasticityTensorCP` is used to create an elasticity tensor for crystal plasticity simulations.
This material builds an orthotropic elasticity tensor using the fill_method `symmetric9` from [ComputeElasticityTensor](/ComputeElasticityTensor.md).
`ComputeElasticityTensorCP` rotates the elasticity tensor both during the initial setup step, if Euler angles are provided, and at the start of each timestep as the crystal lattice rotates during plastic deformation.

The fill method `symmetric9` is appropriate for materials with three orthotropic planes of symmetry \cite{malvern1969introduction}, and is used for simulations of anistropic materials such as cubic crystals.
The enginering elasticity tensor notation for an orthotropic material is given in Eq \eqref{eq:symmetric9_fill_method}:
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
The [Euler angle convention](http://mathworld.wolfram.com/EulerAngles.html) used in `ComputeElasticityTensorCP` is the $z$-$x'$-$z'$ (3-1-3) convention.
The Euler angles arguments are expected in degrees, not radians, and are denoted as $\phi_1$, $\Phi$, and $\phi_2$, corresponding to the axis rotations.
The rotation tensor, $R$, is calculated from the current Euler angles at each timestep as shown in Eq \eqref{eq:rotation_tensor}.
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
The elasticity tensor is then rotated with Eq \eqref{eq:cp_elasticity_tensor_rotation}
\begin{equation}
\label{eq:cp_elasticity_tensor_rotation}
  C'_{ijkl} = R^T_{im} R^T_{jn} R^T_{ko} R^T_{lp} C_{mnop}
\end{equation}
at the beginning of each material timestep calculation.


## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/cp_user_object/crysp.i block=Materials/elasticity_tensor


!syntax parameters /Materials/ComputeElasticityTensorCP

!syntax inputs /Materials/ComputeElasticityTensorCP

!syntax children /Materials/ComputeElasticityTensorCP

## References
\bibliographystyle{unsrt}
\bibliography{tensor_mechanics.bib}
