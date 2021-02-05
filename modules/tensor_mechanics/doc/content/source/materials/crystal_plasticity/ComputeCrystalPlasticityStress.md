# ComputeCrystalPlasticityStress

!syntax description /Materials/ComputeCrystalPlasticityStress

## Description

Among the many fields which study the mechanics of crystalline solids, crystal
plasticity has been established as a capable tool to explore the relationship
between crystalline microstructure evolution and engineering scale stress
response [!citep](Asaro:1983kf, Roters:2010). The formulation of crystal plasticity
within a continuum mechanics framework enables the utilization of these models
for longer time scales and larger length scales than atomistic and dislocations
dynamics models which explicitly track each individual dislocation and defect.

The class `ComputeCrystalPlasticityStress` calls the specified crystal plasticity
constitutive model class and stores the Cauchy stress calculated by the crystal
plasticity model. `ComputeCrystalPlasticityStress` is designed to be used in
conjunction with a crystal plasticity model to calculate the inelastic strain
response.

!alert note title=Requires Specific Constitutive Crystal Plasticity Base Class
Any constitutive crystal plasticity model developed for use with the
`ComputeCrystalPlasticityStress` class *must* inherit from the
`CrystalPlasticityUpdate` base class.

`ComputeCrystalPlasticityStress` computes an initial elastic strain 'trial' value
and a corresponding initial 'trial' Cauchy stress value. These initial values are
passed to the constitutive crystal plasticity model, in which the evolution equations
are  assumed to be implemented in an updated Lagrangian incremental form, through
the `CrystalPlasticityUpdate` base class. Based on the constitutive model definition,
a locally converged stress, see [#newtonRhapson2ndPK], is computed at each simulation
iteration on every mesh quadrature point.


In light of the essential role played by the `CrystalPlasticityUpdate` base class
in the calculation of the strain and Cauchy stress increment, an overview of the
algorithm used to calculate and converge the strain and stress increments for a
general crystal plasticity constitutive model is given below.

## Algorithm in `CrystalPlasticityUpdate`

Within the `CrystalPlasticityUpdate`  base class, the crystal slip and resulting
strain increment are implemented in an updated Lagrangian incremental form. As
such, all constitutive evolution equations should also be written in the Lagrangian
form (and related to the reference frame).

The corresponding second Piola-Kirchoff stress measure is used to determine
local convergence, at each quadrature point, with in the crystal plasticity
constitutive model base class. Once convergence is achieved within the
user-specified tolerances, the equivalent Cauchy stress value is calculated and
passed back to the `ComputeCrystalPlasticityStress` class. The Cauchy stress
measure is used in the traditional FEM residual calculation within the MOOSE
framework.

A brief overview of the relevant continuum mechanics concepts for the crystal
plasticity constitutive model base class is given before the crystal plasticity
specific algorithm overview.


### Deformation Gradient and Strain Measure in the Reference Configuration

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
  \mathbf{F} = \mathbf{R} \cdot \mathbf{U} = \mathbf{V} \cdot \mathbf{R}
\end{equation}
where $\mathbf{R}$ is the orthogonal rotation tensor and $\mathbf{U}$ and
$\mathbf{V}$ are the symmetric right and left stretch tensors, respectively.
[!cite](cauchy1827,green1841) introduced the following deformation tensors
\begin{equation}
  \label{eqn:defLeftCauchyGreenDeformTensor}
  \mathbf{B}^{-1} = \left( \mathbf{F}^{-1} \right)^T \cdot \mathbf{F}^{-1}
\end{equation}
\begin{equation}
  \label{eqn:defRightCauchyGreenDeformTensor}
  \mathbf{C} = \mathbf{F}^T \cdot \mathbf{F}
\end{equation}
where $\mathbf{B}$ and $\mathbf{C}$ are the left and right Cauchy-Green
deformation tensors. These  deformation tensors enable the introduction of the
Green-Saint Venant and the Almansi-Hemel strain tensors. The Lagrangian
Green-Saint Venant strain tensor can be expressed as a function of the right
Cauchy-Green deformation tensor [!citep](slaughter2012linearized)
\begin{equation}
  \label{eqn:defLagrangianStrainRightCauchyGreen}
  \mathbf{E} = \frac{1}{2} \left( \mathbf{C} - \mathbf{I} \right)
\end{equation}
where $\mathbf{I}$ is the rank-2 identity tensor. The Lagrangian strain tensor
enables the calculation of the strain with respect to the reference configuration
as a function of the
deformation gradient.


### Second Piola-Kirchhoff Stress Use in Constitutive Model Definition

In writing a constitutive equation the appropriate, corresponding strain and
stress measures must be used: the work conjugate pairs. The work conjugate to
the Lagrangian strain tensor is the second Piola-Kirchoff stress measure.

Both the first and second Piola-Kirchhoff stress measures are defined on the
reference configuration, although only the second Piola-Kirchhoff stress tensor
is symmetric. The second Piola-Kirchhoff stress tensor can be
defined from the Cauchy stress tensor as
\begin{equation}
  \label{eqn:defSecondPiolaKirchhoffStress}
  \mathbf{S} = J \cdot \mathbf{F}^{-1} \cdot \mathbf{T} \cdot \left( \mathbf{F}^{-1} \right)^T
\end{equation}
where $J$ is the determinant of the deformation gradient, known as the Jacobian
[!citep](khan1995). Note the use of the inverse deformation gradient to relate
the current configuration frame back to the reference frame.

`CrystalPlasticityUpdate` assumes the crystal plasticity constitutive
relationships are written using these measures.

### General Constitutive Equations

Following [!cite](Asaro:1983kf) `CrystalPlasticityUpdate` uses a multiplicative
decomposition of the deformation gradient into elastic and plastic components:
\begin{equation}
  \label{eqn:deformationGradDecomposition}
  \mathbf{F} = \mathbf{F}^e \mathbf{F}^p
\end{equation}
The change in the crystal shape due to dislocation motion is accounted for in
the plastic deformation gradient tensor, $\mathbf{F}^p$, while the elastic
deformation gradient tensor, $\mathbf{F}^e$, accounts for recoverable elastic
stretch and rotations of the crystal lattice. The evolution of the plastic
deformation gradient is given as
\begin{equation}
  \label{eqn:plasticDeformationGradRate}
  \dot{\mathbf{F}}^p = \mathbf{L}^p \mathbf{F}^p
\end{equation}
where $\mathbf{L}^p$ is the plastic velocity gradient. The plastic deformation
gradient rate is used to calculate the increment of the Lagrangian strain,
[eqn:defLagrangianStrainRightCauchyGreen], for a given $\Delta t$ time step.


The plastic velocity gradient is defined as the sum of the slip increments on
all of the slip systems [!citep](Asaro:1983kf).
\begin{equation}
\label{eqn:SumShears}
\mathbf{L^p} = \sum_{\alpha=1}^n \dot{\gamma}^{\alpha} \mathbf{s}_o^{\alpha} \otimes \mathbf{m}_o^{\alpha}
\end{equation}
where $\dot{\gamma}^{\alpha}$ is the slip rate. Note that the slip direction and
slip plane normal unit vectors, $\mathbf{s}_o$ and $\mathbf{m}_o$, are defined
in the reference configuration. This relationship links the continuum framework
to the constitutive models used in crystal plasticity.

As a consequence of the decision to use the slip direction and plane normal unit
vectors from the initial crystal orientation, [eqn:SumShears], the second
Piola-Kirchoff stress is used to determine the applied resolved shear stress.
\begin{equation}
  \label{eqn:appliedShearStress}
  \tau^{\alpha} = \mathbf{S} : \mathbf{s}^{\alpha}_o \otimes \mathbf{m}^{\alpha}_o
\end{equation}


## Newton-Rhapson Interation id=newtonRhapson2ndPK

The order of calculations performed within the `CrystalPlasticityUpdate` base
class is given in flowchart form, [xtalpl_nr_pk2convergence].  Using a Newton
Rhapson approach, outlined below in [xtalpl_constitutive_convergence], we
consider the system converged when the residual of the second Piola-Kirchhoff
stress increments from the current and previous iterations is within tolerances
specified by the user.

The converged Cauchy stress value and the corresponding strain measure are then
calculated, via the inverse of [eqn:defSecondPiolaKirchhoffStress], before being
passed back to `ComputeCrystalPlasticityStress` for the FEM residual calculation
within the MOOSE framework.

!media tensor_mechanics/crystal_plasticity/crystal_plasticity_stress_update_algorithm.png
    id=xtalpl_nr_pk2convergence
    caption=The flowchart for the calculation of the stress and strain measures within the `CrystalPlasticityUpdate` class as implemented in the tensor mechanics module of MOOSE is shown here. The components involved in the Newton-Rhapson iteration are shown in light blue and the components shown in light orange are executed once per MOOSE iteration.
    style=display:block;margin-left:auto;margin-right:auto;width:80%

The Newton-Rhapson iteration algorithm implemented in `CrystalPlasticityUpdate`
separates the iteration over the second Piola-Kirchhoff stress residual
from the physically based constitutive models used to calculate the plastic
velocity gradient, [xtalpl_constitutive_convergence]. The convergence algorithm
for Newton-Rhapson iteration is taken from other approaches already implemented
in MOOSE and is adapted for our crystal plasticity framework.

!media tensor_mechanics/crystal_plasticity/crystal_plasticity_stress_update_constitutive_convergence.png
    id=xtalpl_constitutive_convergence
    caption=The algorithm components of the Newton-Rhapson iteration are shown in light blue and the crystal plasticity constitutive model components, which should be overwritten by a specific constitutive model class inheriting from `CrystalPlasticityUpdate`, are shown in light green.
    style=display:block;margin-left:auto;margin-right:auto;width:80%


Constitutive models are used to calculate the plastic slip rate in classes which
inherit from `CrystalPlasticityUpdate`.

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
configuration, [eqn:SumShears] and [eqn:appliedShearStress],

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/elasticity_tensor

!syntax parameters /Materials/ComputeCrystalPlasticityStress

!syntax inputs /Materials/ComputeCrystalPlasticityStress

!syntax children /Materials/ComputeCrystalPlasticityStress

!bibtex bibliography
