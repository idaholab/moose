# Compute Crystal Plasticity Volumetric Eigenstrain

[ComputeCrystalPlasticityVolumetricEigenstrain](/ComputeCrystalPlasticityVolumetricEigenstrain.md) is designed to accommodate deformations in the crystal lattice due to the presence of voids. This class uses the simplifying assumptions that the voids are perfect spheres and that the population of voids at a single integration point can be described by a single average void radius and an average number density.

The void average characteristics are allowed to both increase and decrease (e.g. from defect coalescence or from porosity reduction) as calculated from a separate material.


## Mathematical Overview

The volume change in the lattice produced by the voids is calculated as
\begin{equation}
  \frac{\Delta V}{V_o} = \frac{4 \pi}{3}r^3 \rho_v
\end{equation}
where $r$ is the void radius and $\rho_v$ is the number density [!cite](was2007).
Consistent with the unit system used in the MOOSE crystal plasticity models, these two void characteristics have units of $mm$ and $1/mm^3$, respectively.

The equivalent linear strain measure is calculated in the traditional fashion by taking the cubic root of the volume change due to the voids as defined above. The equivalent linear strain increment is defined as the difference between the volume change at two timesteps.
\begin{equation}
  \Delta l = \left[ \left(\frac{\Delta V}{V_o}\bigg\vert_t\right)^{1/3}  - \left(\frac{\Delta V}{V_o}\bigg\vert_{(t-1)}\right)^{1/3} \right]
\end{equation}
where $t$ indicates the current timestep and $(t-1)$ denotes the previous timestep.
The linear expansion increment tensor is then calculated by multiplying the equivalent linear strain increment by the Rank-2 identity tensor, $\mathbf{I}$
\begin{equation}
  \label{eqn:linearMeasureVolumeChangeCP}
  \Delta \boldsymbol{\epsilon}^v = \mathbf{I} \cdot \Delta l.
\end{equation}


As is described in [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md), the eigenstrain contribution is accounted for through the multiplicative decomposition of the deformation gradient:
\begin{equation}
  \label{eqn:F_decomposition}
  \boldsymbol{F} = \boldsymbol{F}^e \boldsymbol{F}^p \boldsymbol{F}^v,
\end{equation}
where the superscripts $e$, $p$, and $v$ denote the elastic, plastic, and volumetric eigenstrain deformation gradients, respectively [!cite](meissonnier2001finite).
The volumetric eigenstrain deformation gradient is calculated as a function of the linear expansion increment tensor, [eqn:linearMeasureVolumeChangeCP]; the linear expansion increment is first rotated to the current configuration.

\begin{equation}
  \mathbf{F}^v = \left( \boldsymbol{\epsilon}^R_v \right)^{-1} \mathbf{F}^v_{old} \qquad \text{where} \qquad \boldsymbol{\epsilon}^R_v = \mathbf{I} - \Delta \boldsymbol{\epsilon}^v \mathbf{R}
\end{equation}
where $\mathbf{F}^v_{old}$ is the volumetric eigenstrain deformation gradient from the previous timestep, $\mathbf{R}$ is the rotation tensor.



## Numerical Implementation

For details of the time integration, Jacobian calculation, and elasto-plastic tangent moduli computation, see the discussion in
[ComputeCrystalPlasticityThermalEigenstrain](/ComputeCrystalPlasticityThermalEigenstrain.md).
These calculations are based on the eigenstrain deformation gradient: replace the thermal deformation gradient, $\boldsymbol{F}^{\theta}$, with the volumetric eigenstrain deformation gradient, $\boldsymbol{F}^v$, to obtain the expressions relevant for
[ComputeCrystalPlasticityVolumetricEigenstrain](/ComputeCrystalPlasticityVolumetricEigenstrain.md).

!alert note title=Base Class Requirement
Any constitutive eigenstrain model developed for use within the [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) class must inherit from the ComputeCrystalPlasticityEigenstrainBase class.


## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/crystal_plasticity/cp_eigenstrains/volumetric_eigenstrain_increase.i block=Materials/stress/ Materials/void_eigenstrain/ Materials/void_density/ Materials/void_radius

!syntax parameters /Materials/ComputeCrystalPlasticityVolumetricEigenstrain

!syntax inputs /Materials/ComputeCrystalPlasticityVolumetricEigenstrain

!syntax children /Materials/ComputeCrystalPlasticityVolumetricEigenstrain
