# UpdatedLagrangianStressDivergence

!syntax description /Kernels/UpdatedLagrangianStressDivergence

## Description

The `UpdatedLagrangianStressDivergence` kernel calculates the stress equilibrium
residual in the current configuration using the `cauchy_stress` (the
Cauchy stress).  This kernel provides the residual
for Cartesian coordinates and the user needs to add one kernel
for each dimension of the problem.  Alternatively, the
[TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md)
simplifies the process of adding the required kernels and setting up the
input parameters.

## Residual, Jacobian, and stabilization

For large deformation kinematics the kernel applies the residual
\begin{equation}
      R^{\alpha}=\int_{v}\sigma_{ik}\phi_{i,k}^{\alpha}dv
\end{equation}
with the corresponding Jacobian
\begin{equation}
      J^{\alpha\beta}=\int_{v}\left\{ T_{ijkl}\phi_{i,j}^{\alpha}f_{km}g_{ml}^{\beta}+\sigma_{ij}\left(\phi_{k,k}^{\alpha}\psi_{ij}^{\beta}-\phi_{k,j}^{\alpha}\psi_{ik}^{\beta}\right)\right\} dv
\end{equation}
where $\sigma_{ik}$ is the Cauchy stress,
\begin{equation}
      T_{ijkl} = \frac{\partial \sigma_{ij}}{\partial \Delta l_{kl}}
\end{equation}
with
\begin{equation}
      \Delta l_{kl} = \Delta F_{kM} F^{-1}_{Ml}
\end{equation}
the incremental spatial velocity gradient,
$\phi_{i,j}^{\alpha}$ are the test function gradients (with respect to the current
coordinates) and
\begin{equation}
     g_{ij}^{\beta} = \frac{dF_{iK}}{d\Upsilon^{\beta}} F_{Kj}^{-1}
\end{equation}
with $\Upsilon^\beta$ the discrete (nodal) displacements.
For the unstabilized case
\begin{equation}
     g_{ij}^{\beta} =  \psi_{i,j}^{\beta}
\end{equation}
with $\psi_{i,j}^{\beta}$ the trial function gradients with respect to the current coordinates.

The residual and Jacobian degenerate to
\begin{equation}
      R^{\alpha}=\int_{v}s_{ij}\phi_{i,j}^{\alpha}dv
\end{equation}
and
\begin{equation}
      J^{\alpha\beta}=\int_{V}\phi_{i,j}^{\alpha}C_{ijkl}g_{kl}^{\beta}dV
\end{equation}
for the small deformation case, with $s_{ij}$ the small stress,
\begin{equation}
      C_{ijkl} = \frac{\partial s_{ij}}{\partial \varepsilon_{kl}}
\end{equation}
with $\varepsilon_{kl}$ the small strain and
\begin{equation}
      g_{kl}^{\beta} = \psi_{k,l}^{\beta}
\end{equation}
for the unstabilized case.
The `large_kinematics` flag controls the kinematic theory.
For this kernel `use_displaced_mesh` must be set to `true` if
`large_kinematics` is `true` so that the volume integrals and
gradients are with respect to the current coordinates.

For the Jacobian the small deformation
\begin{equation}
      \phi_{i,j}^{\alpha}C_{ijkl}g_{kl}^{\beta}
\end{equation}
term and the large deformation term
\begin{equation}
      T_{ijkl}\phi_{i,j}^{\alpha}f_{km}g_{ml}^{\beta}
\end{equation}
are identical (assuming the incremental deformation gradient is the identity and the deformation gradient
degenerates to the small strain for the small deformation case) and involve
the constitutive model.  These are then called the "material" part of the Jacobian.  The remaining
large deformation term
\begin{equation}
      \sigma_{ij}\left(\phi_{k,k}^{\alpha}\psi_{ij}^{\beta}-\phi_{k,j}^{\alpha}\psi_{ik}^{\beta}\right)
\end{equation}
involves the current stress and the updated geometry and so it is called the "geometric" part of the
Jacobian.

The constitutive model needs to provide the Cauchy stress and the derivative of
that stress with respect to the increment in the spatial velocity gradient.
However, the [material system](tensor_mechanics/NewMaterialSystem.md)
provides a common interface to define the constitutive model with any stress and strain
measures that are convient, translating the user-defined stress and Jacobian to the correct
form automatically.
Note that if the model is rotationally invariant then
\begin{equation}
      \Delta l_{kl} = \Delta d_{kl}
\end{equation}
where $\Delta d_{kl}$ is the increment in the material deformation rate, equal to the increment
in the logarthmic strain [!cite](freed2014).

The kernel is compatible with the [$\bar{\boldsymbol{F}}$ modification](/tensor_mechanics/Stabilization.md) of the
strains to stabilize the problem for incompressible or nearly incompressible deformation.
This form of stabilization does not modify the residual equation, though the modified strain does change the constitutive
model stress update.
The strain modification does affect the Jacobian by altering the definition of the gradient tensors.  With the
modified strains applied these become for small deformations
\begin{equation}
      g_{ij}^{\beta}=\psi_{i,j}^{\beta}-\frac{1}{3}\left(\psi_{kk}^{\beta}-\bar{\psi}_{kk}^{\beta}\right)\delta_{ij}
\end{equation}
with
\begin{equation}
      \bar{\psi}_{i,j}^{\beta}=\frac{1}{v}\int_{v}\psi_{i,j}^{\beta}dv
\end{equation}
and for large deformations
\begin{equation}
      g_{iJ}^{\beta}=\left[\psi_{i,j}^{\beta}-\frac{1}{3}\delta_{ij}\left(\psi_{k,k}^{\beta}-\bar{\psi}_{k,k}^{\beta}\right)\right]
\end{equation}
with
\begin{equation}
      \bar{\psi}_{i,j}^{\beta}=\frac{1}{V}\int_{V}\psi_{i,K}^{\beta}dV\bar{F}_{Kj}^{-1}
\end{equation}
and $\bar{F}$ the average deformation gradient, defined in the [stabilization system documentation](/tensor_mechanics/Stabilization.md).
Note this is a somewhat unusual integral for an updated Lagrangian model, but it follows to keep the
derivative term consistent with the $\bar{\boldsymbol{F}}$ modification to the strains.
The `stabilize_strain` flag controls if the kernel modifies the Jacobian to account for the stabilized strains.

## `use_displaced_mesh`

The `UpdatedLagrangianStressDiverence` kernel is the only object in the new, Lagrangian Tensor Mechanics system
that requires `use_displaced_mesh` to be set.  The `use_displaced_mesh` flag should be set to `true` if and only
if `large_kinematics` is also `true`.  The kernel enforces this condition with an error.

## Example Input File Syntax

The following illustrates manually including 3D stress equilibrium with the total Lagrangian formulation, using
large deformation kinematics.

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/updated/patch/large_patch.i
         block=Kernels

!syntax parameters /Kernels/UpdatedLagrangianStressDivergence

!syntax inputs /Kernels/UpdatedLagrangianStressDivergence

!syntax children /Kernels/UpdatedLagrangianStressDivergence

!bibtex bibliography
