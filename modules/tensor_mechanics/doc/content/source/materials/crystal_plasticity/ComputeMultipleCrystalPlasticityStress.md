# Compute Multiple Crystal Plasticity Stress

!syntax description /Materials/ComputeMultipleCrystalPlasticityStress

## Description

Among the many fields which study the mechanics of crystalline solids, crystal
plasticity has been established as a capable tool to explore the relationship
between crystalline microstructure evolution and engineering scale stress
response [!citep](Asaro:1983kf, Roters:2010). The formulation of crystal plasticity
within a continuum mechanics framework enables the utilization of these models
for longer time scales and larger length scales than atomistic and dislocations
dynamics models which explicitly track each individual dislocation and defect.

The `ComputeMultipleCrystalPlasticityStress` class is designed to facilitate the
implementation of different crystal plasticity constitutive laws with a certain degree of
modularity among models (e.g., flexibility to include one or several slip modes).
Comparing to the existing [FiniteStrainUObasedCP](/FiniteStrainUObasedCP.md) class,
`ComputeMultipleCrystalPlasticityStress` relies on the `Materials` system instead
of the `UserObject` system, to store and update state variables. This means for
a specific constitutive model, the computation of variables, e.g., slip rate,
slip resistance, state variables, and the associated rate components will all be self-contained
in one material class. As a result, this base class greatly reduces the number of additional
classes that is necessary for a new crystal plasticity constitutive model. Meanwhile, the input
block that defines a specific crystal plasticity model is also significantly simplified.

The class `ComputeMultipleCrystalPlasticityStress` is designed to be used in conjunction with
one or several crystal plasticity models that are derived from the `CrystalPlasticityStressUpdateBase` class.


!alert note title=Requires Specific Constitutive Crystal Plasticity Base Class
Any constitutive crystal plasticity model developed for use with the
`ComputeMultipleCrystalPlasticityStress` class *must* inherit from the
`CrystalPlasticityStressUpdateBase` class.

## Crystal Plasticity Governing Equations

Within the `ComputeMultipleCrystalPlasticityStress`  base class, the crystal slip and resulting
strain increment are implemented in an updated Lagrangian incremental form. As
such, all constitutive evolution equations should also be written in the Lagrangian
form (and related to the reference frame).

The corresponding second Piola-Kirchoff stress measure is used to determine
local convergence, at each quadrature point, with in the crystal plasticity
constitutive model base class. Once convergence is achieved within the
user-specified tolerances, the equivalent Cauchy stress value is calculated.
The Cauchy stress measure is then used in the traditional FEM residual calculation within the MOOSE
framework (see [xtalpl_nr_pk2convergence]).

A brief overview of the relevant continuum mechanics concepts for the crystal
plasticity constitutive model base class is given below.

### Constitutive Equations

For finite strain inelastic mechanics of crystal plasticity the deformation gradient $\boldsymbol{F}$ is assumed to be multiplicatively decomposed in its elastic and plastic parts as (see e.g., [!cite](Asaro:1983kf)):
\begin{equation}
\boldsymbol{F} = \boldsymbol{F}^e \boldsymbol{F}^p,
\end{equation}
such that $\text{det}\left( \boldsymbol{F}^e \right) > 0$ and $\text{det}\left( \boldsymbol{F}^p \right) = 1$ are the elastic and plastic deformation gradients, respectively.
 The elastic and plastic deformation gradients define two deformed configurations: one intermediate configuration described by $\boldsymbol{F}^{p}$, and a final deformed configuration $\boldsymbol{F}^e \boldsymbol{F}^p$. Here, the change in the crystal shape due to dislocation motion is accounted for in
 the plastic deformation gradient tensor, $\boldsymbol{F}^p$, the elastic
 deformation gradient tensor, $\boldsymbol{F}^e$, accounts for recoverable elastic
 stretch and rotations of the crystal lattice,

 For a thermo-mechanical problem, a third configuration is introduced accounting for thermal deformations (see e.g., [!cite](li2019development,ozturk2016crystal,meissonnier2001finite)). The resulting decomposition reads
\begin{equation}
  \label{eqn:deformationGradDecomposition}
  \boldsymbol{F} = \boldsymbol{F}^e \boldsymbol{F}^p \boldsymbol{F}^{\theta},
\end{equation}
where $\boldsymbol{F}^{\theta}$ is the thermal deformation gradient, such that  $\text{det}\left( \boldsymbol{F}^{\theta} \right) > 0$.
The thermal deformation gradient $\boldsymbol{F}^{\theta}$ accounts for the deformation of the crystal lattice due to thermal expansion.
The constitutive equation and calculation details associated with the thermal deformation gradient $\boldsymbol{F} ^{\theta}$ are included in [ComputeCrystalPlasticityThermalEigenstrain](/ComputeCrystalPlasticityThermalEigenstrain.md).

The total plastic velocity gradient can be expressed in terms of the plasticity deformation gradient as
\begin{equation}
  \label{eqn:plasticDeformationGradRate}
  \boldsymbol{L^p} =\dot{\boldsymbol{F}}^p \boldsymbol{F}^{p-1}.
\end{equation}
This formulation is used to compute the updated plastic deformation gradient via backward time integration, i.e.,
\begin{equation}
\label{eqn:fpinv_update}
  \boldsymbol{F}^{p-1}_{n+1} = \boldsymbol{F}^{p-1}_{n}\left( \boldsymbol{I} -  \boldsymbol{L^p} \Delta t  \right),
\end{equation}
where $\boldsymbol{I}$ is the second order identity tensor, the subscript $(n+1)$ denotes the current step, $(n)$ denotes the previous step, and $\Delta t$ is the time step size.
The plastic deformation gradient is used to calculate the elastic deformation gradient via [eqn:deformationGradDecomposition], which is
then used in the computation of the elastic Lagrangian strain.
The elastic Lagrangian strain $\boldsymbol{E}^e$ is defined as
\begin{equation}
\label{eqn:Ee}
    \boldsymbol{E}^e = \frac{1}{2} \left(  {\boldsymbol{F}^e}^{\intercal} \boldsymbol{F}^e  - \boldsymbol{I} \right).
\end{equation}
Let $\boldsymbol{S}$ be the second Piola-Kirchhoff tensor, the stress that is work conjugate to $\boldsymbol{E}^e$. An elastic constitutive relation is given by
\begin{equation}
\label{eqn:stress_pk2}
    \boldsymbol{S} = \mathbb{C} : \boldsymbol{E}^e,
\end{equation}
where $\mathbb{C}$ is the fourth-order temperature dependent elasticity tensor.
The stress $\boldsymbol{S}$ is the pull back of the Cauchy stress ($\boldsymbol{\sigma}$) from the current configuration
which is,
\begin{equation}
\label{eqn:stress_cauchy}
\boldsymbol{S} = \text{det}(\boldsymbol{F}^e) \boldsymbol{F}^{e-1} \boldsymbol{\sigma} \left( \boldsymbol{F}^{e-1} \right)^\intercal
\end{equation}

To account for multiple deformation mechanisms, the total plastic velocity gradient ($\boldsymbol{L^p}$) is
computed as the sum of plastic velocity gradients coming from
each crystal plasticity constitutive equations,
\begin{equation}
\label{eqn:SumShears_multiple_cp}
\boldsymbol{L^p} = \sum_{i=1}^m \mathcal{L^p_i},
\end{equation}
where $\mathcal{L^p_i}$ is the plastic velocity gradient corresponding to th $i$th crystal plasticity deformation mechanism,
$m$ is the total number of deformation modes. Here, $\mathcal{L^p_i}$  is defined as the sum of the slip increments on
all of the slip systems [!citep](Asaro:1983kf),
\begin{equation}
\label{eqn:SumShears_one_cp}
\mathcal{L}^p_i = \sum_{\alpha=1}^n \dot{\gamma}_i^{\alpha} \boldsymbol{s}_{i,o}^{\alpha} \otimes \boldsymbol{m}_{i,o}^{\alpha}
\end{equation}
where $\dot{\gamma}^{\alpha}_i$ is the slip rate for the $i$ the model on the slip system $\alpha$. Note that the associated slip direction and
slip plane normal unit vectors, $\boldsymbol{s}_{i,o}$ and $\boldsymbol{m}_{i,o}$, are defined
in the reference configuration.
The evolution of the plastic slip $\gamma^\alpha_i$ must be specified for every slip plane and every slip mechanism, which can be expressed as
\begin{equation}
\label{eqn:gamma_dot}
    \dot{\gamma}^\alpha_{i} = \hat{\dot{\gamma}}^\alpha_i \left( \tau^\alpha_i, s^\alpha_i \right),
\end{equation}
which can take different forms among constitutive models used in crystal plasticity. Here, the $s^\alpha_i$ denotes the slip resistance and $\tau^\alpha_i$ denotes the resolved shear stress that is associated with slip system $\alpha$ and model $i$, respectively.

The resolved shear stress is defined as
\begin{equation}
  \label{eqn:appliedShearStress}
  \tau^\alpha = \text{det}(\boldsymbol{F}^\theta) \left(  {\boldsymbol{F}^{\theta}}^{\intercal} \boldsymbol{S} {\boldsymbol{F}^{\theta}}^{-\intercal} \right) : \boldsymbol{s}^{\alpha}_{i,o} \otimes \boldsymbol{m}^{\alpha}_{i,o}.
\end{equation}
Here, we report the shear stress in a form that considers the thermal effect. For readers who are particularly interested in the thermal eigenstrain calculations, please refer to the  [ComputeCrystalPlasticityThermalEigenstrain](/ComputeCrystalPlasticityThermalEigenstrain.md) documentation page.

To facilitate the understanding of the above constitutive equations, some fundamental continuum mechanics concepts are included in the following subsections.

#### Deformation Gradient and Strain Measure in the Reference Configuration

The deformation gradient can be used to find the deformation in the reference
configuration as
\begin{equation}
  \label{eqn:defDeformationGradientInverse}
  X_{K,i} = F^{-1}_{iK} = \frac{\partial X_K}{\partial x_i} = \delta_{Ki} + u_{K,i}
\end{equation}
where an upper case subscript denotes the reference configuration, a lower case
subscript denotes the current configuration, and $u$ denotes the displacement.

The deformation gradient can be multiplicatively decomposed via the Cauchy
theorem for nonsingular tensors into two rank-2 tensors, one representing the
stretch and the other rotation motion [!citep](malvern1969introduction),
\begin{equation}
  \label{eqn:defRUDecomponsition}
  \boldsymbol{F} = \boldsymbol{R} \cdot \boldsymbol{U} = \boldsymbol{V} \cdot \boldsymbol{R}
\end{equation}
where $\boldsymbol{R}$ is the orthogonal rotation tensor and $\boldsymbol{U}$ and
$\boldsymbol{V}$ are the symmetric right and left stretch tensors, respectively.
[!cite](cauchy1827,green1841) introduced the following deformation tensors
\begin{equation}
  \label{eqn:defLeftCauchyGreenDeformTensor}
  \boldsymbol{B}^{-1} = \left( \boldsymbol{F}^{-1} \right)^T \cdot \boldsymbol{F}^{-1}
\end{equation}
\begin{equation}
  \label{eqn:defRightCauchyGreenDeformTensor}
  \boldsymbol{C} = \boldsymbol{F}^T \cdot \boldsymbol{F}
\end{equation}
where $\boldsymbol{B}$ and $\boldsymbol{C}$ are the left and right Cauchy-Green
deformation tensors. These  deformation tensors enable the introduction of the
Green-Saint Venant and the Almansi-Hemel strain tensors. The Lagrangian
Green-Saint Venant strain tensor can be expressed as a function of the right
Cauchy-Green deformation tensor [!citep](slaughter2012linearized)
\begin{equation}
  \label{eqn:defLagrangianStrainRightCauchyGreen}
  \boldsymbol{E} = \frac{1}{2} \left( \boldsymbol{C} - \boldsymbol{I} \right)
\end{equation}
where $\boldsymbol{I}$ is the second order identity tensor. The Lagrangian strain tensor
enables the calculation of the strain with respect to the reference configuration
as a function of the deformation gradient. In this `ComputeMultipleCrystalPlasticityStress` class, the elastic part of the
Lagrangian strain tensor ($\boldsymbol{E}^e$) is utilized in the calculation of stress (see [eqn:Ee] and [eqn:stress_pk2]).

#### Second Piola-Kirchhoff Stress

In writing a constitutive equation the appropriate, corresponding strain and
stress measures must be used: the work conjugate pairs. The work conjugate to
the Lagrangian strain tensor is the second Piola-Kirchoff stress measure.

Both the first and second Piola-Kirchhoff stress measures are defined on the
reference configuration, although only the second Piola-Kirchhoff stress tensor
is symmetric. The second Piola-Kirchhoff stress tensor can be
defined from the Cauchy stress tensor as
\begin{equation}
  \label{eqn:defSecondPiolaKirchhoffStress}
  \boldsymbol{S} = \text{det}(\boldsymbol{F}) \cdot \boldsymbol{F}^{-1} \cdot \boldsymbol{\sigma} \cdot \left( \boldsymbol{F}^{-1} \right)^\intercal
\end{equation}
The inverse deformation gradient is used to relate
the current configuration frame back to the reference frame (i.e., pull back). Note in the `ComputeMultipleCrystalPlasticityStress` class,
the pull back of Cauchy stress is via the elastic part of the deformation gradient ($\boldsymbol{F}^e$) only (see [eqn:stress_cauchy]).

### Calculation of Schmid Tensor

The calculation of the flow direction Schmid tensor, the dyadic product of the slip direction and slip plane normal unit vectors, $\boldsymbol{s}_{i,o}^{\alpha} \otimes \boldsymbol{m}_{i,o}^{\alpha}$, is straight forward for the case of cubic crystals, including Face Centered Cubic (FCC) and Body Centered Cubic (BCC) crystals. The 3-index Miller indices commonly used to describe the slip direction and slip plane normals are first normalized individually normalized and then directly used in the dyadic product.

#### Conversion of Miller-Bravais Indices for HCP to Cartesian System

Hexagonal Close Packed (HCP) crystals are often described with the 4-index Miller-Bravais system:
\begin{equation}
  \label{eqn:millerBravaisHCPIndices}
  (HKIL) [UVTW]
\end{equation}
To compute the Schmid tensor from these slip direction and slip plane normals, the indices must first be transformed to the Cartesian coordinate system. Within the associated `ComputeMultipleCrystalPlasticityStress` implementation, this conversion uses the assumption that the a$_1$-axis, or the H index, align with the x-axis in the basal plane of the HCP crystal lattice, see [xtalpl_hcp_basalplane_notation]. The c-axis, the L index, is assumed to be paralled to the z-axis of the Cartesian system.

!media tensor_mechanics/crystal_plasticity/HCP_basal_plane_diagram.png
    id=xtalpl_hcp_basalplane_notation
    caption=The convention used to transform the 4-index Miller-Bravais indices to the 3-index Cartesian system aligns the x-axis with the a$_1$-axis in the basal plane in this implementation.
    style=display:block;margin-left:auto;margin-right:auto;width:40%

The slip plane directions are transformed to the Cartesian system with the matrix equation
\begin{equation}
  \label{eqn:hcpSlipDirectionTransform}
  \begin{bmatrix}
    \frac{1}{a} & 0 & 0 \\
      &  & \\
    \frac{1}{a\sqrt{3}} & \frac{2}{a\sqrt{3}} & 0 \\
      &  & \\
    0 &    0  & \frac{1}{c}
  \end{bmatrix}   \cdot
  \begin{bmatrix}
    U \\
           \\
    V \\
           \\
    W
  \end{bmatrix}_{|hex} =
  \begin{bmatrix}
    x \\
    y \\
    z
  \end{bmatrix}_{|cart}
\end{equation}

where a and c are the HCP unit cell lattice parameters in the basal and axial directions, respectively.
A check is performed with the basal plane indices to ensure that those indices sum to zero ($U + V + W = 0$).
If the slip direction indices are given as decimal values, nummerical round-off errors may require an increase in the value of the parameter `zero_tol` which is used within the code to set the allowable deviation from exact zero.

The slip plane normals are similiarly transformed as
\begin{equation}
  \label{eqn:hcpSlipPlaneNormalTransform}
  \begin{bmatrix}
    \frac{1}{a} & 0 & 0 \\
      &  & \\
    \frac{1}{a\sqrt{3}} & \frac{2}{a\sqrt{3}} & 0 \\
      &  & \\
    0 &    0  & \frac{1}{c}
  \end{bmatrix}   \cdot
  \begin{bmatrix}
    H \\
      \\
    K \\
      \\
    L
  \end{bmatrix}_{|hex} =
  \begin{bmatrix}
    h \\
    k \\
    l
  \end{bmatrix}_{|cart}
\end{equation}

Once transformed to the Cartesian system, these vectors are normalized and then used to compute the Schmid tensor.

!alert note
The alignment of the a$_1$ axis of the Miller-Bravais notation and the x-axis of the Cartesian system within the basal plane of the unit HCP is specifically adopted for the conversion implementation in the `ComputeMultipleCrystalPlasticityStress` associated classes. While there is broad consensus in the alignment of the HPC c-axis with the Cartesian z-axis, no standard for alignment within the basal plane is clear. Users should note this assumption in the construction of their simulations and the interpretations of the simulation results.

## Calculation of Crystal Rotation

The rotation of a crystal during deformation is calculated within the `ComputeMultipleCrystalPlasticityStress` class through a polar decomposition on the elastic part of the deformation tensor ($\boldsymbol{F}^{e}$), i.e.,

\begin{equation}
  \boldsymbol{F}^{e} = \boldsymbol{R}^{e}\cdot\boldsymbol{U}^{e},
\end{equation}
where $\boldsymbol{U}^{e}$ is the symmetric matrix that describes the elastic stretch of the crystal, $\boldsymbol{R}^{e}$ is the orthogonal tensor that describes the elastic part of the crystal rotation.

Note here $\boldsymbol{R}^{e}$ represents the rotation of the crystal with respect to its crystal lattice. To obtain the the crystal rotation relative to the reference frame (total rotation $\boldsymbol{R}^{\text{total}}$), initial orientation of the crystal needs to be considered, i.e.,

\begin{equation}
  \boldsymbol{R}^{\text{total}} = \boldsymbol{R}^{e}\cdot\boldsymbol{R}^{\text{initial}},
\end{equation}
where $\boldsymbol{R}^{\text{initial}}$ denotes the rotation matrix that corresponds to the initial orientation of the crystal.

To obtain the Euler angles for the crystals during deformation, one will need to transform the rotation matrix to the Euler angle. One can refer to the [ComputeUpdatedEulerAngle](/ComputeUpdatedEulerAngle.md) class for this purpose.

## Computation Workflow

The order of calculations performed within the `ComputeMultipleCrystalPlasticityStress`
class is given in flowchart form, [xtalpl_nr_pk2convergence].  The converged Cauchy
stress value and the corresponding strain measure are then
calculated, via the inverse of [eqn:defSecondPiolaKirchhoffStress], before continuing
the FEM residual calculation within the MOOSE framework.


!media tensor_mechanics/crystal_plasticity/crystal_plasticity_stress_update_algorithm.png
    id=xtalpl_nr_pk2convergence
    caption=The flowchart for the calculation of the stress and strain measures within the `ComputeMultipleCrystalPlasticityStress` class as implemented in the tensor mechanics module of MOOSE is shown here. The components involved in the Newton-Rhapson iteration are shown in light blue and the components shown in light orange are executed once per MOOSE iteration.
    style=display:block;margin-left:auto;margin-right:auto;width:80%

Using a Newton Rhapson approach, outlined below in [xtalpl_constitutive_convergence], we
consider the system converged when the residual of the second Piola-Kirchhoff
stress increments from the current and previous iterations is within tolerances
specified by the user.

The Newton-Rhapson iteration algorithm implemented in `CrystalPlasticityStressUpdateBase`
separates the iteration over the second Piola-Kirchhoff stress residual
from the physically based constitutive models used to calculate the plastic
velocity gradient, see [xtalpl_constitutive_convergence]. The convergence algorithm
for Newton-Rhapson iteration is taken from other approaches already implemented
in MOOSE and is adapted for our crystal plasticity framework.

!media tensor_mechanics/crystal_plasticity/multiple_crystal_plasticity_stress_update_constitutive_convergence.png
    id=xtalpl_constitutive_convergence
    caption=The algorithm components of the Newton-Rhapson iteration are shown in light blue and the crystal plasticity constitutive model components, which should be overwritten by a specific constitutive model class inheriting from `CrystalPlasticityStressUpdateBase`, are shown in light green.
    style=display:block;margin-left:auto;margin-right:auto;width:80%


Constitutive models are used to calculate the plastic slip rate in classes which
inherit from `CrystalPlasticityStressUpdateBase`.



## Units Assumed in the Crystal Plasticity Materials

The simulation domain for crystal plasticity models is resolved on the order of
individual crystal grains, and, as such, the mesh size is small. Although MOOSE
itself is dimension agnostic, the crystal plasticity models are implemented in
the +mm-MPa-s unit system+. This dimension system choice impacts the
input files in the following manner:

- Mesh dimensions should be constructed in mm
- Elastic constant values (e.g. Young's modulus and shear modulus) are entered in MPa
- Initial slip system strength values are entered in MPa
- Simulation times are given in s
- Strain rates and displacement loading rates are given in 1/s and mm/s, respectively

In physically based models, which maybe based on this class, initial densities
of crystal defects (e.g. dislocations, point defects) should be given in
1/mm$^2$ or 1/mm$^3$

## Example Input File

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/stress

Note that the specific constitutive crystal plasticity model must also be given
in the input file to define the plastic slip rate increment

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/trial_xtalpl

Finally a specific constant elasticity tensor class must be used with these materials
to account for the use of the slip planes and directions in the reference
configuration, [eqn:SumShears_multiple_cp], [eqn:SumShears_one_cp], and [eqn:appliedShearStress],

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/elasticity_tensor

!syntax parameters /Materials/ComputeMultipleCrystalPlasticityStress

!syntax inputs /Materials/ComputeMultipleCrystalPlasticityStress

!syntax children /Materials/ComputeMultipleCrystalPlasticityStress

!bibtex bibliography
