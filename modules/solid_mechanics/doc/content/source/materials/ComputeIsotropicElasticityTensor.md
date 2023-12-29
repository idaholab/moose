# Compute Isotropic Elasticity Tensor

!syntax description /Materials/ComputeIsotropicElasticityTensor

## Description

The material `ComputeIsotropicElasticityTensor` builds the isotropic elasticity (stiffness) tensor with two user provided elastic constants.

The isotropic elasticity tensor is given, in engineering matrix notation [!citep](malvern1969introduction), as
\begin{equation}
\label{eq:isotropic_fill_method}
C_{ijkl}^{isotropic} = \begin{bmatrix}
              \lambda + 2 \mu & \lambda & \lambda &      0 &      0 &      0 \\
              \lambda & \lambda + 2 \mu & \lambda &      0 &      0 &      0 \\
              \lambda & \lambda & \lambda + 2 \mu &      0 &      0 &      0 \\
                   0 &      0 &      0 &    \mu &      0 &      0 \\
                   0 &      0 &      0 &      0 &    \mu &      0 \\
                   0 &      0 &      0 &      0 &      0 &    \mu
              \end{bmatrix}
\end{equation}

`ComputeIsotropicElasticityTensor` accepts as an argument two of five isotropic elastic constants: lambda $\lambda$, the shear modulus $\mu$, the bulk modulus $K$, the Young's modulus $E$, or the Poisson's ratio $\nu$.
The material includes the conversions into Lame constants, see [!cite](slaughter2012linearized) for the conversion equations among the isotropic elastic constants.

An automatic differentiation version of this object is available as `ADComputeIsotropicElasticityTensor`.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch_quadratic.i block=Materials/elast_tensor

!syntax parameters /Materials/ComputeIsotropicElasticityTensor

!syntax inputs /Materials/ComputeIsotropicElasticityTensor

!syntax children /Materials/ComputeIsotropicElasticityTensor

!bibtex bibliography
