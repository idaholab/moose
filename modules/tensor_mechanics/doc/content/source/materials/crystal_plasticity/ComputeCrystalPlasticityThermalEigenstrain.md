# Compute Crystal Plasticity Thermal Eigenstrain


As is described in [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md), a thermal deformation gradient $\boldsymbol{F}^{\theta}$ is introduced to account for the thermal induced deformations in a finite strain thermo-mechanical problem with crystal plasticity. Accordingly, the
total deformation gradient is multiplicatively decomposed into three components as
\begin{equation}
  \label{eqn:F_decomposition}
  \boldsymbol{F} = \boldsymbol{F}^e \boldsymbol{F}^p \boldsymbol{F}^{\theta},
\end{equation}
such that $\text{det}\left( \boldsymbol{F}^e \right) > 0$, $\text{det}\left( \boldsymbol{F}^p \right) = 1$, and  $\text{det}\left( \boldsymbol{F}^{\theta} \right) > 0$ (see [!cite](li2019development,ozturk2016crystal,meissonnier2001finite)).

To account for more than one thermal eigenstrain, one can optionally decompose the total thermal deformation gradient $\boldsymbol{F}^{\theta}$ into multiple components as
\begin{equation}
  \label{eqn:Ftheta_decomposition}
  \boldsymbol{F}^{\theta} = \boldsymbol{F}^1 \boldsymbol{F}^2 \cdots \boldsymbol{F}^{N},
\end{equation}
where a total of $N$ different thermal eigenstrains are considered.

The thermal Lagrangian strain that is associated with $\boldsymbol{F}^{N}$ is computed as
\begin{equation}
  \label{eqn:FN_eigenstrain}
  \boldsymbol{E}^{N} = \frac{1}{2}\left({\boldsymbol{F}^{N}}^{\intercal} \boldsymbol{F}^{N}   - \boldsymbol{I}\right).
\end{equation}

In this class, the evolution of a typical thermal deformation gradient component, $\boldsymbol{F}^N$ (see [eqn:Ftheta_decomposition]) is expressed with respect to its lattice symmetry axis as
\begin{equation}
\label{eqn:FN}
    \dot{ \boldsymbol{F}} ^{N} {(\boldsymbol{F}^{N})}^{-1} = \dot{\theta} \boldsymbol{\beta},
\end{equation}
where $\dot{\theta}$ is the temperature rate, and  $\boldsymbol{\beta} = \text{diag} \left( \beta_1, \beta_2, \beta_3 \right)$ is a diagonal tensor for anisotropic thermal expansion coefficients.
One can either use the above evolution equation, or create a different crystal plasticity thermal eigenstrain class with a customized constitutive equation by inheriting from the ComputeCrystalPlasticityEigenstrainBase class.


!alert note title=Base Class Requirement
Any constitutive eigenstrain model developed for use within the [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) class must inherit from the ComputeCrystalPlasticityEigenstrainBase class.

We summarize the numerical implementation details that are specialized in crystal plasticity thermal eigenstrain computation in the following.


## Numerical Implementation

The calculations of the thermal deformation gradient $\boldsymbol{F}^{\theta}$ (or its component $\boldsymbol{F}^{N}$), the Lagrangian strain $\boldsymbol{E}^{\theta}$ (or $\boldsymbol{E}^{N}$), and the associated derivative $\partial \boldsymbol{F}^{\theta} / \partial \theta$ (or $\partial \boldsymbol{F}^{N} / \partial \theta$) are included in this [ComputeCrystalPlasticityThermalEigenstrain](/ComputeCrystalPlasticityThermalEigenstrain.md) class.
Meanwhile, changes are required in the residual and Jacobian calculations in [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) in order to account for the additional configuration due to thermal expansion. The implementation details are described below.

### 1. Time integration for $\boldsymbol{F}^{N}$

The evolution of the thermal deformation gradient component $\boldsymbol{F} ^{N}$ is defined by [eqn:FN]. Similar to the time integration that has been implemented for $\boldsymbol{F}^p$, we start with a backward difference expression as an approximation for $\boldsymbol{F}^N$,
\begin{equation}
    {\boldsymbol{F}^{N}_{n+1}} \approx \frac{ \boldsymbol{F} ^{N}_{n+1} - \boldsymbol{F} ^{N}_{n} }{\Delta t},
\end{equation}
where the subscripts $n$ and $n+1$ identifies the time steps. By substituting into [eqn:FN] and rearranging, we have
\begin{equation}
     {(\boldsymbol{F}^{N}_{n+1})}^{-1} = {(\boldsymbol{F}^{N}_{n})}^{-1} \left( \boldsymbol{I} - \Delta \theta \boldsymbol{\beta} \right)
\end{equation}
where $\Delta \theta = \dot{\theta} \Delta t$.

### 2. Crystal plasticity Jacobian

The stress is updated iteratively by driving the stress residual to zero. The stress residual is computed by
\begin{equation}
    \mathbb{R} = \boldsymbol{S} - \mathbb{C} : \boldsymbol{E}^e,
\end{equation}
where $\boldsymbol{S}$ is the second Piola-Kirchhoff stress, the $\mathbb{C}$ is the elasticity tensor, and $\boldsymbol{E}^e$ is the elastic part of the Cauchy-Green deformation tensor.
The crystal plasticity Jacobian is computed by taking the derivative of $\mathbb{R}$ with respect to $\boldsymbol{S}$. By using the chain rule, we have
\begin{equation}
\label{eqn:Jacobian}
    \mathbb{J} = \mathbb{I} -  \mathbb{C} : \left\{\frac{\partial\boldsymbol{E}^e}{\partial\boldsymbol{F}^e}\frac{\partial\boldsymbol{F}^e }{\partial{\boldsymbol{F}^p}^{-1}}\frac{\partial{\boldsymbol{F}^p}^{-1}}{\partial\boldsymbol{S}}\right\}.
\end{equation}
The computation of each term in the curly bracket will be described in the follows using indicial notations. The ${\partial\boldsymbol{E}^e_{ij}}/{\partial \boldsymbol{F}^e_{kl}}$ is computed as
\begin{equation*}
\frac{\partial \boldsymbol{E}^e_{ij}}{ \partial \boldsymbol{F}^e_{kl}} = \frac{1}{2} \delta_{il} \boldsymbol{F}^e_{kj} + \frac{1}{2} \delta_{jl} {\boldsymbol{F}^e_{ik}}^{\intercal}.
\end{equation*}

From [eqn:F_decomposition], $\boldsymbol{F}^e_{ij} = \boldsymbol{F}_{im} {\boldsymbol{F}^{\theta}}^{-1}_{mn} {\boldsymbol{F}^{p}}^{-1}_{nj}$. Therefore,
${\partial \boldsymbol{F}^e_{ij} }/{\partial{\boldsymbol{F}^p}^{-1}_{kj}}=\boldsymbol{F}_{im}{\boldsymbol{F}^{\theta}}^{-1}_{mk}$.

we evaluate the last term in [eqn:Jacobian] via chain rule:
\begin{equation}
    \frac{\partial {\boldsymbol{F}^p}^{-1} }{\partial \boldsymbol{S}} = \sum_\alpha \left\{  \frac{\partial {\boldsymbol{F}^p}^{-1}}{\partial \gamma^{\alpha} } \frac {\partial \gamma^{\alpha} }{\partial \tau^\alpha} \frac{\partial \tau^\alpha}{\partial  \boldsymbol{S}}  \right\},
\end{equation}
where the evaluation of ${\partial {\boldsymbol{F}^p}^{-1}}/{\partial \gamma^{\alpha} }$ is based on $\boldsymbol{L^p} =\dot{\boldsymbol{F}}^p \boldsymbol{F}^{p-1}$ by taking chain rule. The evaluation of ${\partial \gamma^{\alpha} } / {\partial \tau^\alpha}$ varies based on the form of the flow rule that differs for different material types. The last term is evaluated as
\begin{equation}
     \frac{\partial \tau^\alpha}{\partial  \boldsymbol{S}} = \text{det}(\boldsymbol{F}^{\theta}) {\boldsymbol{F}^\theta} \boldsymbol{S}_{o}^{\alpha} {\boldsymbol{F}^{\theta}}^{-1}
\end{equation}
where $\boldsymbol{S}^{\alpha}_{o}$ is the schmid tensor for slip system $\alpha$.

### 3. Elasto-plastic tangent moduli

The elasto-plastic tangent moduli is computed via
\begin{equation}
    \frac{\partial\boldsymbol{\sigma}}{ \partial \boldsymbol{F}} = \left(  \frac{\partial \boldsymbol{\sigma}}{\partial \boldsymbol{S}} \frac{\partial \boldsymbol{S}}{\partial \boldsymbol{F}^{e}} + \frac{\partial \boldsymbol{\sigma}}{\partial \boldsymbol{F}^e} \right) \frac{\partial \boldsymbol{F}^e}{\partial \boldsymbol{F}},
\end{equation}
where the ${\partial \boldsymbol{\sigma}}/{\partial \boldsymbol{S}}$, ${\partial \boldsymbol{S}}/{\partial \boldsymbol{F}^{e}}$ and  ${\partial \boldsymbol{\sigma}}/{\partial \boldsymbol{F}^e}$ remains the same with the inclusion of the thermal deformation gradient. The last term is evaluated as
\begin{equation}
    \frac{\partial \boldsymbol{F}^e_{ij}}{\partial \boldsymbol{F}_{il}} =  \boldsymbol{F}^{\theta -1 }_{ln} \boldsymbol{F}^{p -1 }_{nj}.
\end{equation}

!alert note title=Off-diagonal Jacobian
The off-diagonal Jacobian contributions due to thermal expansion is currently *not* accounted for in the stress divergence calculation. Improving the Jacobian entries associated with thermal expansion remains an ongoing work.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/multiple_eigenstrains_test.i block=Materials/stress/ Materials/thermal_eigenstrain_1/ Materials/thermal_eigenstrain_2/

!syntax parameters /Materials/ComputeCrystalPlasticityThermalEigenstrain

!syntax inputs /Materials/ComputeCrystalPlasticityThermalEigenstrain

!syntax children /Materials/ComputeCrystalPlasticityThermalEigenstrain
