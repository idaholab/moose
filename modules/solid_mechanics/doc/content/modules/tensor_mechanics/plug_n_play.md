# Plug-n-Play System Overview in Tensor Mechanics

The tensor mechanics materials use a plug-and-play system where the main tensors used in the residual
equation are defined in individual material classes in MOOSE. The plug-and-play approach used in the
Tensor Mechanics module requires at least three separate classes to fully describe a material model.

!alert note title=Three Tensors Are Required for a Mechanics Problem
The three tensors that must be defined for any mechanics problem are the the strain
$\boldsymbol{\epsilon}$ or strain increment, elasticity tensor $\boldsymbol{\mathcal{C}}$, and the stress
$\boldsymbol{\sigma}$. Optional tensors include stress-free strain (also known as an eigenstrain)
$\boldsymbol{\epsilon}_0$ and additional stress $\boldsymbol{\sigma}_0$.

!media media/tensor_mechanics/IntroPlugNPlay.png
       style=width:800;float:right;
       caption=Figure 1: Tensors required to fully describe a mechanics material.

At times, a user may need to define multiple mechanics properties over a single block. For this
reason, all material properties can be prepended by a name defined by the input parameter
`base_name`.

## Strain Materials

The base material class to create strains ($\boldsymbol{\epsilon}$) or strain increments is
`ComputeStrainBase`; this class is a pure virtual class, requiring that all children override the
`computeQpProperties()` method.  For all strains the base class defines the property `total_strain`.
For incremental strains, both finite and small, the compute strain base class defines the properties
`strain_rate`, `strain_increment`, `rotation_increment`, and `deformation_gradient`. A discussion of
the different types of strain formulations is available on the [Strains](tensor_mechanics/Strains.md)
page.

For small strains, use [ComputeSmallStrain](/ComputeSmallStrain.md) in which $\boldsymbol{\epsilon} =
(\nabla \boldsymbol{u} + \nabla \boldsymbol{u}^T)/2$. For finite strains, use
[ComputeFiniteStrain](/ComputeFiniteStrain.md) in which an incremental form is employed such that the
strain_increment and rotation_increment are calculated.

With the TensorMechanics master action, the strain formulation can be set with the `strain= SMALL |
FINITE` parameter, as shown below.

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i
         block=Modules/TensorMechanics

## Elasticity Tensor Materials

The primary class for creating elasticity tensors ($\boldsymbol{\mathcal{C_{ijkl}}}$) is
[ComputeElasticityTensor](/ComputeElasticityTensor.md). This class defines the property
`_elasticity_tensor`. Given the elastic constants required for the applicable symmetry, such as
`symmetric9`, this material calculates the elasticity tensor. If you wish to rotate the elasticity
tensor, constant Euler angles can be provided. The elasticity tensor can also be scaled with a
function, if desired.  `ComputeElasticityTensor` also serves as a base class for specialized
elasticity tensors, including:

- An elasticity tensor for crystal plasticity, [ComputeElasticityTensorCP](/ComputeElasticityTensorCP.md),
- A Cosserat elasticity tensor [ComputeLayeredCosseratElasticityTensor](/ComputeLayeredCosseratElasticityTensor.md),
- An isotropic elasticity tensor [ComputeIsotropicElasticityTensor](/ComputeIsotropicElasticityTensor.md).

The input file syntax for the isotropic elasticity tensor is

!listing modules/tensor_mechanics/tutorials/basics/part_1.1.i block=Materials/elasticity_tensor

and for an orthotropic material, such as a metal crystal, is

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i
         block=Materials/elasticity_tensor

## Stress Materials

The base class for constitutive equations to compute a stress ($\boldsymbol{\sigma}$) is
`ComputeStressBase`. The `ComputeStressBase` class defines the properties `stress` and
`elastic_strain`. It is a pure virtual class, requiring all children to override the method
`computeQpStress()`.

Two elastic constitutive models have been developed, one that assumes small strains
[ComputeLinearElasticStress](/ComputeLinearElasticStress.md), and a second which assumes finite
strains and rotations increments
[ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md) The input file syntax for
these materials is

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i
         block=Materials/stress

There are a number of other constitutive models that have been implemented to calculate more complex
elasticity problems, plasticity, and creep.  An overview of these different materials is available on
the [Stresses](tensor_mechanics/Stresses.md) page.

## Eigenstrain Materials

Eigenstrain is the term given to a strain which does not result directly from an applied force. The
base class for eigenstrains is `ComputeEigenstrainBase`. It computes an eigenstrain, which is
subtracted from the total strain in the Compute Strain classes.
\begin{equation}
\epsilon_{mechanical} = \epsilon_{total} - \epsilon_{eigen}
\end{equation}
Chapter 3 of [!cite](qu2006fundamentals) describes the relationship between total, elastic, and eigen- strains and
provides examples using thermal expansion and dislocations.

Eigenstrains are also referred to as residual strains, stress-free strains, or intrinsic strains;
translated from German, [Eigen](http://dict.tu-chemnitz.de/deutsch-englisch/Eigen....html) means own
or intrinsic in English.  The term eigenstrain was introduced by
[!cite](mura1982general):

> Eigenstrain is a generic name given to such nonelastic strains as thermal expansion, phase
> transformation, initial strains, plastic, misfit strains. Eigenstress is a generic name given to
> self-equilibrated internal stresses caused by one or several of these eigenstrains in bodies which
> are free from any other external force and surface constraint.  The eigenstress fields are created
> by the incompatibility of the eigenstrains.  This new English terminology was adapted from the
> German "Eigenspannungen and Eigenspannungsquellen," which is the title of H. Reissner's paper
> (1931) on residual stresses.

Thermal strains are a volumetric change resulting from a change in temperature of the material.  The
change in strains can be either a simple linear function of thermal change,
e.g. ($\boldsymbol{\epsilon}_T = \alpha \Delta T$) or a more complex function of temperature.  The
thermal expansion class, [ComputeThermalExpansionEigenstrain](/ComputeThermalExpansionEigenstrain.md)
computes the thermal strain as a linear function of temperature.  The input file syntax is

!listing modules/tensor_mechanics/test/tests/thermal_expansion/constant_expansion_coeff.i
         block=Materials/thermal_expansion_strain

The eigenstrain material block name must also be added as an input parameter, `eigenstrain_names` to
the strain material or TensorMechanics master action block. An example of the additional parameter in
the TensorMechanics master action is shown below.

!listing modules/tensor_mechanics/test/tests/thermal_expansion/constant_expansion_coeff.i
         block=Modules/TensorMechanics

Other eigenstrains could be caused by defects such as over-sized or under-sized second phase
particles. Such an eigenstrain material is
[ComputeVariableEigenstrain](/ComputeVariableEigenstrain.md). This class computes a lattice mismatch
due to a secondary phase, where the form of the tensor is defined by an input vector, and the scalar
dependence on a phase variable is defined in another material. The input file syntax is

!listing modules/combined/test/tests/eigenstrain/inclusion.i block=Materials/eigenstrain

Note the `DerivativeParsedMaterial`, which evaluates an expression given in the input file, and its
automatically generated derivatives, at each quadrature point.

!listing modules/combined/test/tests/eigenstrain/inclusion.i block=Materials/var_dependence

## Extra Stress Materials

Extra stresses ($\boldsymbol{\sigma}_0$) can also be pulled into the residual calculation after the
constitutive model calculation of the stress. The extra stress material property, `extra_stress` is
defined in the `ComputeExtraStressBase` class and is added to the stress value.

\begin{equation}
  \sigma_{ij} = \sigma_{ij} + \sigma^{extra}_{ij}
\end{equation}

An extra stress may be a residual stress, such as in large civil engineering simulations.  An example
of an extra stress material used in an input file is:

!listing modules/combined/test/tests/linear_elasticity/extra_stress.i block=Materials/const_stress

!bibtex bibliography
