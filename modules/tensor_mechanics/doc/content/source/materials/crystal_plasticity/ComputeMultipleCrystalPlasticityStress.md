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

Following what is typical in crystal plasticity computations (e.g., [!cite](Asaro:1983kf)),
the total deformation gradient $\boldsymbol{F}$ is multiplicatively decomposed into
elastic ($\boldsymbol{F}^e$) and plastic ($\boldsymbol{F}^{p}$) components:

\begin{equation}
  \label{eqn:deformationGradDecomposition}
  \boldsymbol{F} = \boldsymbol{F}^e \boldsymbol{F}^p,
\end{equation}
such that $\text{det}\left( \boldsymbol{F}^e \right) > 0$ and $\text{det}\left( \boldsymbol{F}^p \right) = 1$.
The change in the crystal shape due to dislocation motion is accounted for in
the plastic deformation gradient tensor, $\boldsymbol{F}^p$, while the elastic
deformation gradient tensor, $\boldsymbol{F}^e$, accounts for recoverable elastic
stretch and rotations of the crystal lattice.

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
which can take different forms among constitutive models used in crystal plasticity. Here, the $s^\alpha_i$ denotes the slip resistance and $\tau^\alpha_i$ denotes the resolved shear stress that is associated with slip system $\alpha$ and model $i$, respectively. The resolved shear stress is defined as
\begin{equation}
  \label{eqn:appliedShearStress}
  \tau^{\alpha}_i = \boldsymbol{S} : \boldsymbol{s}^{\alpha}_{i,o} \otimes \boldsymbol{m}^{\alpha}_{i,o}.
\end{equation}

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

## Computation workflow

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
