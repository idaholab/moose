# Rank Two Scalar Aux
!syntax description /AuxKernels/RankTwoScalarAux

##Description
The AuxKernel `RankTwoScalarAux` provides methods to calculate several different scalar quantities for a Rank-2 tensor, described below.
In some types of calculations, the scalar quantity is calculated in a user-specified direction (by default set to the (0, 0, 1) axis); in other calculation types the user can specify the start and end points of a line along which the scalar quantity is calculated.
Quantities commonly examined with `RankTwoScalarAux` are stress ($\boldsymbol{\sigma}$) and strain ($\boldsymbol{\epsilon}$).

If desired, `RankTwoScalarAux` can be restricted to calculate the scalar quantity data for a Rank-2 tensor at a single specified quadrature point per element. This option is generally used only for debugging purposes.

## Axial Stress
The scalar type `AxialStress` calculates the scalar value of a Rank-2 tensor, $T$, in the direction of the axis specified by the user.
The user should give the starting point, $P^1$, and the end point, $P^2$ which define the axis.

\begin{equation}
\label{eq:axial_stress_scalar_type}
s = \hat{a}_i T_{ij} \hat{a}_j \quad \text{ where } \quad \hat{a}_i = \frac{P^2_i - P^1_i}{\left| P^2_i - P^1_i \right|}
\end{equation}
where $\hat{a}$ is the normalized direction vector for the axis defined by the points $P^1$ and $P^2$.

## Direction
The scalar type `direction` calculates the scalar value of a Rank-2 tensor, $T$, in the direction selected by the user as shown by Eq \eqref{eq:direction_scalar_type}:
\begin{equation}
\label{eq:direction_scalar_type}
s = D_i T_{ij} D_j
\end{equation}
where $D$ is the direction vector specified in the input file.

### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxKernels/direction

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxVariables/direction

## Effective Strain
The scalar type `EffectiveStrain` calculates an effective scalar measure of a Rank-2 tensor, often strain ($\epsilon_{ij}$) according to Eq
\eqref{eq:effective_strain_scalar_type}.
\begin{equation}
\label{eq:effective_strain_scalar_type}
s = \sqrt{\frac{2}{3} \epsilon_{ij} \epsilon_{ij}}
\end{equation}

!!! warning "This Strain Measure is not for Inelastic Strains"
    This effective strain measure, `EffectiveStrain`, should not be confused with the effective plastic strain or effective creep strain, which are computed as integrals over the history of the inelastic strain; these effective inelastic strain measures are computed as scalar material properties (named `effective_plastic_strain` and `effective_creep_strain`) for applicable $J_2$ models. The effective inelastic strains are computed as:
    \begin{equation}
    s = \int_t\sqrt{\frac{2}{3} \dot{\epsilon}^p_{ij} \dot{\epsilon}^p_{ij}} \mathrm{d}t
    \end{equation}
    where $t$ is time and $\dot{\epsilon}^p_{ij}$ is the inelastic strain increment.

### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/auxkernels/ranktwoscalaraux.i block=AuxKernels/peeq

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/auxkernels/ranktwoscalaraux.i block=AuxVariables/peeq

## Hoop Stress
The scalar type `HoopStress` calculates the value of a Rank-2 tensor along the hoop direction of a cylinder, shown in Eq \eqref{eq:hoop_stress_scalar_type}.
The cylinder is defined with a normal vector from the current position to the cylinder surface and a user specified axis of rotation.
The user defines this rotation axis with a starting point, $P^1$, and the end point, $P^2$.

\begin{equation}
\label{eq:hoop_stress_scalar_type}
s = \hat{n}^h_i T_{ij} \hat{n}^h_j
\end{equation}
where $\hat{n}^h$ is the hoop direction normal, defined as
\begin{equation}
\label{eq:hoop_direction_normal}
  \begin{aligned}
    \hat{n}^h & = \hat{n}^c \times \hat{a} \\
    \hat{n}^c & = \frac{P^c - \hat{n}^r}{\left| P^c - \hat{n}^r \right|} \\
    \hat{a} & = \frac{P^2 - P^1}{\left| P^2 - P^1 \right|}
  \end{aligned}
\end{equation}
where $P^c$ is the current sampling position point, and $\hat{n}^r$ is the direction normal to the plane defined by the cylinder axis of rotation vector and the direction normal to the axis of rotation at the current position $P^c$.

### Example Input File Syntax
!listing modules/combined/test/tests/axisymmetric_2d3d_solution_function/2d.i block=AuxKernels/hoop_stress

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/combined/test/tests/axisymmetric_2d3d_solution_function/2d.i block=AuxVariables/hoop_stress

## Hydrostatic Stress
The scalar type `Hydrostatic` calculates the hydrostatic scalar of a Rank-2 tensor, $T_{ij}$, as shown in Eq \eqref{eq:hydrostatic_scalar_type}.
\begin{equation}
\label{eq:hydrostatic_scalar_type}
s = \frac{Tr \left( T_{ij} \right)}{3} = \frac{T_{ii}}{3}
\end{equation}

### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/material_limit_time_step/creep/nafems_test5a_lim.i block=AuxKernels/pressure

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/material_limit_time_step/creep/nafems_test5a_lim.i block=AuxVariables/pressure


## Invariant Values
### First Invariant
The scalar type `FirstInvariant` calculates the first invariant of the specified Rank-2 tensor, $T_{ij}$, according to Eq \eqref{eq:first_invariant_scalar_type} from \cite{malvern1969introduction}.
\begin{equation}
\label{eq:first_invariant_scalar_type}
I_T = Tr \left( T_{ij} \right) = T_{ii}
\end{equation}

#### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxKernels/fi

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxVariables/firstinv

### Second Invariant
Similarly, the scalar type `SecondInvariant` finds the second invariant of the Rank-2 tensor, $T_{ij}$, as shown in Eq \eqref{eq:second_invariant_scalar_type}
This method is defined in \cite{hjelmstad2007fundamentals}.
\begin{equation}
\label{eq:second_invariant_scalar_type}
II_T = T_{ii} T_{jj} - \frac{1}{2} \left( T_{ij} T_{ij} + T_{ji} T_{ji} \right)
\end{equation}

#### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxKernels/si

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxVariables/secondinv

### Third Invariant
The scalar type `ThirdInvariant` computes the value of the Rank-2 tensor, $T_ij$, third invariant as given in Eq \eqref{eq:third_invariant_scalar_type} from \cite{malvern1969introduction}.
\begin{equation}
\label{eq:third_invariant_scalar_type}
III_T = det \left( T_{ij} \right)  = \frac{1}{6} e_{ijk} e_{pqr} T_{ip} T_{jq} T_{kr}
\end{equation}
where $e$ is the Rank-3 permutation tensor.

#### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxKernels/ti

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxVariables/thirdinv


## L2 Norm
The scalar type `L2Norm` calculates the L2 normal of a Rank-2 tensor, $T_{ij}$, as shown in Eq \eqref{eq:l2_norm_scalar_type}.
\begin{equation}
\label{eq:l2_norm_scalar_type}
s = \sqrt{T_{ij} T_{ij}}
\end{equation}

## Principal Values
### Maximum Principal Quantity
The scalar type `MaxPrincipal` calculates the largest principal value for a symmetric tensor, using the calcEigenValues method from the Rank Two Tensor utility class.

#### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxKernels/maxprincipal

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxVariables/maxprincipal

### Middle Principal Quantity
Similarly, the scalar type `MidPrincipal` finds the second largest principal value for a symmetric tensor, using the calcEigenValues method from the Rank Two Tensor utility class.

#### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxKernels/midprincipal

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxVariables/midprincipal

### Minimum Principal Quantity
The scalar type `MinPrincipal` computes the smallest principal value for a symmetric tensor, using the calcEigenValues method from the Rank Two Tensor utility class.

#### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxKernels/minprincipal

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxVariables/minprincipal


## Radial Stress
The scalar type `RadialStress` calculates the scalar component for a Rank-2 tensor, $T_{ij}$, in the direction of the normal vector from the user-defined axis of rotation, as shown in Eq \eqref{eq:radial_stress_scalar_type}.
\begin{equation}
\label{eq:radial_stress_scalar_type}
s = \hat{n}^r_i T_{ij} \hat{n}^r_j
\end{equation}
where $\hat{n}^r$ is the direction normal to the plane defined by the cylinder axis of rotation vector and the direction normal to the axis of rotation at the current position $P^c$.

### Example Input File Syntax
!listing modules/porous_flow/test/tests/thm_rehbinder/free_outer.i block=AuxKernels/stress_rr

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/porous_flow/test/tests/thm_rehbinder/free_outer.i block=AuxVariables/stress_rr

## Triaxiality Stress
The scalar type `TriaxialityStress` finds the ratio of the hydrostatic measure, $T_{hydrostatic}$, to the von Mises measure, $T_{vonMises}$, as shown in Eq \eqref{eq:triaxiality_scalar_type}.
As the name suggests, this scalar measure is most often used for stress tensors.
\begin{equation}
\label{eq:triaxiality_scalar_type}
s = \frac{T_{hydrostatic}}{T_{vonMises}} = \frac{\frac{1}{3} T_{ii}}{\sqrt{\frac{3}{2} S_{ij} S_{ij}}}
\end{equation}
where $S_{ij}$ is the deviatoric tensor of the Rank-2 tensor $T_{ij}$.

## Volumetric Strain
The scalar type `VolumetricStrain` computes the change in volume divided by the original volume and is most appliable for strain.
For a Rank-2 tensor, $T_{ij}$, the volumetric strain quantity is calculated with Eq \eqref{eq:volumetric_strain_scalar_type}.
\begin{equation}
\label{eq:volumetric_strain_scalar_type}
s = T_{11} \cdot \left( T_{22} + T_{33} \right) + T_{22} \cdot T_{33} + T_{11} \cdot T_{22} \cdot T_{33}
\end{equation}

### Example Input File Syntax
!listing modules/combined/test/tests/internal_volume/rz_displaced.i block=AuxKernels/fred

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/combined/test/tests/internal_volume/rz_displaced.i block=AuxVariables/volumetric_strain

## Von Mises Stress
The scalar type `VonMisesStress` calculates the vonMises measure for a Rank-2 tensor, as shown in Eq \eqref{eq:vonmises_scalar_type}.
This quantity is usually applied to the stress tensor.
\begin{equation}
\label{eq:vonmises_scalar_type}
s = \sqrt{\frac{3}{2} S_{ij} S_{ij}}
\end{equation}
where $S_{ij}$ is the deviatoric tensor of the Rank-2 tensor $T_{ij}$.

### Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i block=AuxKernels/vonmises

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.
!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i block=AuxVariables/vonmises

As with the [RankTwoAux](/RankTwoAux.md) AuxKernel, `RankTwoScalarAux` requires the inclusion of an AuxVariable block for each AuxKernel block.

## AuxVariable Order

!!! info "Elemental vs Nodal Visualization of Quadrature Field Values"
    Using an AuxVariable with `family = MONOMIAL` and `order = CONSTANT` will give a constant value of the AuxVariable for the entire element, which is computed by taking a volume-weighted average of the integration point quantities. Using an AuxVariable with `family = MONOMIAL` and `order = FIRST` or higher will result in fields that vary linearly (or with higher order) within each element, which are computed using a local least-squares procedure. Because the Exodus mesh format does not support higher-order elemental variables, these AuxVariables are output by libMesh as nodal variables for visualization purposes. Using higher order monomial variables in this way can produce smoother visualizations of results for a properly converged simulation.

!syntax parameters /AuxKernels/RankTwoScalarAux

!syntax inputs /AuxKernels/RankTwoScalarAux

!syntax children /AuxKernels/RankTwoScalarAux

## References
\bibliographystyle{unsrt}
\bibliography{tensor_mechanics.bib}
