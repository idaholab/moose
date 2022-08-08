# TotalLagrangianStressDivergence

!syntax description /Kernels/TotalLagrangianStressDivergence

## Description

The `TotalLagrangianStressDivergence` kernel calculates the stress equilibrium
residual in the reference configuration using the `pk1_stress` (the
1st Piola-Kirchhoff stress).  This kernel provides the residual
for Cartesian coordinates and the user needs to add one kernel
for each dimension of the problem.  Alternatively, the
[TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md)
simplifies the process of adding the required kernels and setting up the
input parameters.

## Residual, Jacobian, and stabilization

For large deformation kinematics the kernel applies the residual giving the weak
form of the divergence of the 1st Piola Kirchhoff stress with respect to the
reference coordinates
\begin{equation}
      R^{\alpha}=\int_{V}P_{iK}\phi_{i,K}^{\alpha}dV
\end{equation}
with the corresponding Jacobian
\begin{equation}
      J^{\alpha\beta}=\int_{V}\phi_{i,J}^{\alpha}T_{iJkL}^{\prime}G_{kL}^{\beta}dV
\end{equation}
where $P_{iK}$ is the first Piola-Kirchhoff stress,
\begin{equation}
      T_{iJkL}^{\prime}=\frac{dP_{iJ}}{dF_{kL}}
\end{equation}
$\phi_{i,J}^{\alpha}$ are the test function gradients (with respect to the reference
coordinates) and
\begin{equation}
      G_{iJ}^{\beta}=\frac{dF_{iJ}}{d\Upsilon^{\beta}}
\end{equation}
with $\Upsilon^\beta$ the discrete (nodal) displacements.
For the unstabilized case
\begin{equation}
     G_{ij}^{\beta} =  \psi_{i,J}^{\beta}
\end{equation}
with $\psi_{i,J}^{\beta}$ the trial function gradients with respect to the reference coordinates.

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

The constitutive model needs to provide the first Piola-Kirchhoff stress and the derivative of
that stress with respect to the deformation gradient.
However, the [material system](tensor_mechanics/NewMaterialSystem.md)
provides a common interface to define the constitutive model with any stress and strain
measures that are convient, translating the user-defined stress and Jacobian to the correct
form automatically.

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
     G_{iJ}^{\beta} = \left(\frac{\det\bar{F}}{\det F}\right)^{1/3}\left[\psi_{i,J}^{\beta}-\frac{1}{3}F_{iJ}\left(F_{Lk}^{-1}\psi_{k,L}^{\beta}-\bar{F}_{Lk}^{-1}\bar{\psi}_{k,L}^{\beta}\right)\right]
\end{equation}
with
\begin{equation}
    \bar{\psi}_{i,J}^{\beta}=\frac{1}{V}\int_{V}\psi_{i,J}^{\beta}dV
\end{equation}
and $\bar{F}$ the average deformation gradient, defined in the [stabilization system documentation](/tensor_mechanics/Stabilization.md).

## Example Input File Syntax

The following illustrates manually including 3D stress equilibrium with the total Lagrangian formulation, using
large deformation kinematics.

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/total/patch/large_patch.i
         block=Kernels

!syntax parameters /Kernels/TotalLagrangianStressDivergence

!syntax inputs /Kernels/TotalLagrangianStressDivergence

!syntax children /Kernels/TotalLagrangianStressDivergence
