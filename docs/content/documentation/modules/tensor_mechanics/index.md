# TensorMechanics Module

* [Complete System List](tensor_mechanics/systems.md)
* [Strains](tensor_mechanics/Strains.md)
* [Stresses](tensor_mechanics/Stresses.md)
* [Stress Divergence](tensor_mechanics/StressDivergence.md)
* [Visualizing Tensors](tensor_mechanics/VisualizingTensors.md)
* [Tensor Classes](tensor_mechanics/TensorClasses.md)
* [Convergence Criteria](tensor_mechanics/Convergence.md)
* [Volumetric locking correction](tensor_mechanics/VolumetricLocking.md)
* [Fracture Integrals](tensor_mechanics/FractureIntegrals.md)

The MOOSE tensor mechanics module is a library for simplifying the implementation of simulation tools that solve mechanics problems. It provides a simple approach for implementing even advanced mechanics models.

The tensor mechanics module provides a powerful system for solving solid mechanics problems using a simple to use syntax based on tensor forms. This approach allows the tensor equations to be implemented clearly and concisely.

##Mathematical Introduction

A material varies from its rest shape due to stress. This departure from the rest shape is called deformation or displacement, and the proportion of deformation to original size is called strain. To determine the deformed shape and the stress, a governing equation is solved to determine the displacement vector $\mathbf{u}$.

The strong form of the governing equation on the domain $\Omega$ and boundary $\Gamma=\Gamma_{\mathit{t_i}}\cup\Gamma_{\mathit{g_i}}$
can be stated as follows:
\begin{equation}
\begin{aligned}
\nabla \cdot (\mathbf{\sigma} + \mathbf{\sigma}_0) + \mathbf{b} =& \mathbf{0} \;\mathrm{in}\;\Omega \\
\mathbf{u} =& \mathbf{g}\;\mathrm{in}\;\Gamma_{ \mathbf{g}} \\
\mathbf{\sigma} \cdot \mathbf{n}=&\mathbf{t}\;\mathrm{in}\;\Gamma_{ \mathbf{t}}
\end{aligned}
\end{equation}
where $\mathbf{\sigma}$  is the Cauchy stress tensor, $\mathbf{\sigma}_0$ is an additional source of stress (such as pore pressure), $\mathbf{u}$ is the displacement vector, $\mathbf{b}$ is the body force, $\mathbf{n}$ is the unit normal to the boundary, $\mathbf{g}$ is the prescribed displacement on the boundary and $\mathbf{t}$ is the prescribed traction on the boundary. The weak form of the residual equation is expressed as:
\begin{equation}
  \mathbb{R} = \left( \mathbf{\sigma} + \mathbf{\sigma}_0), \nabla \phi_m \right) - \left< \mathbf{t}, \phi_m \right> - \left( \mathbf{b}, \phi_m \right)  = \mathbf{0}
\end{equation}
where $(\cdot)$ and $\left< \cdot \right>$ represent volume and boundary integrals, respectively. The solution of the residual equation with Newton's method requires the Jacobian of the residual equation, which can be expressed as (ignoring boundary terms)
\begin{equation}
  \mathbb{J} = \left( \frac{\partial \mathbf{\sigma}}{\partial \nabla \mathbf{u}} , \nabla \phi_m \right),
\end{equation}
assuming $\mathbf{\sigma}_0$ is independent of the strain.

The material stress response is described by the constitutive model, where the stress is determined as a function of the strain, i.e. $\tilde{\mathbf{\sigma}}( \mathbf{\epsilon} - \mathbf{\epsilon}_0)$, where $\mathbf{\epsilon}$ is the strain and $\mathbf{\epsilon}_0$ is a stress free strain. For example, in linear elasticity (only valid for small strains), the material response is linear, i.e.
$\mathbf{\sigma} = \mathbf{\mathcal{C}}(\mathbf{\epsilon} - \mathbf{\epsilon}_0)$.

The tensor mechanics system can handle linear elasticity and finite strain mechanics, including elasticity, plasticity, creep, and damage.

## Using Materials in Tensor Mechanics

The tensor mechanics materials use a modular system where the main tensors used in the residual equation are defined in individual material classes in MOOSE. The plug-and-play approach used in the Tensor Mechanics module requires at least three separate classes to fully describe a material model.

!!! info "Three Tensors Are Required for a Mechanics Problem"
    The three tensors that must be defined for any mechanics problem are the the strain $\mathbf{\epsilon}$ or strain increment, elasticity tensor $\mathbf{\mathcal{C}}$, and the stress $\mathbf{\sigma}$. Optional tensors include stress-free strain (also known as an eigenstrain) $\mathbf{\epsilon}_0$ and additional stress $\mathbf{\sigma}_0$.

!media media/tensor_mechanics-IntroPlugNPlay.png width=800 float=right caption=Figure 1: Tensors required to fully describe a mechanics material.


At times, a user may need to define multiple mechanics properties over a single block. For this reason, all material properties can be prepended by a name defined by the input parameter `base_name`.

##Strain Materials
The base material class to create strains ($\mathbf{\epsilon}$) or strain increments is `ComputeStrainBase`; this class is a pure virtual class, requiring that all children override the `computeQpProperties()` method.
For all strains the base class defines the property `total_strain`.  For incremental strains, both finite and small, the compute strain base class defines the properties
`strain_rate`, `strain_increment`, `rotation_increment`, and `deformation_gradient`. A discussion of the different types of strain formulations is available on the [Strains](tensor_mechanics/Strains.md) page.

For small strains, use [ComputeSmallStrain](/ComputeSmallStrain.md) in which $\mathbf{\epsilon} = (\nabla \mathbf{u} + \nabla \mathbf{u}^T)/2$. For finite strains, use [ComputeFiniteStrain](/ComputeFiniteStrain.md) in which an incremental form is employed such that the strain_increment and rotation_increment are calculated.

With the TensorMechanics master action, the strain formulation can be set with the `strain= SMALL | FINITE` parameter, as shown below.
!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Modules/TensorMechanics


##Elasticity Tensor Materials

The primary class for creating elasticity tensors ($\mathbf{\mathcal{C_{ijkl}}}$) is [ComputeElasticityTensor](/ComputeElasticityTensor.md). This class defines the property
`_elasticity_tensor`. Given the elastic constants required for the applicable symmetry, such as `symmetric9`, this material calculates the elasticity tensor. If you wish to rotate the elasticity tensor, constant Euler angles can be provided. The elasticity tensor can also be scaled with a function, if desired.
`ComputeElasticityTensor` also serves as a base class for specialized elasticity tensors, including:

* An elasticity tensor for crystal plasticity, [ComputeElasticityTensorCP](/ComputeElasticityTensorCP.md),
* A Cosserat elasticity tensor [ComputeLayeredCosseratElasticityTensor](/ComputeLayeredCosseratElasticityTensor.md),
* An isotropic elasticity tensor [ComputeIsotropicElasticityTensor](/ComputeIsotropicElasticityTensor.md).

The input file syntax for the isotropic elasticity tensor is

!listing modules/tensor_mechanics/tutorials/basics/part_1.1.i block=Materials/elasticity_tensor

and for an orthotropic material, such as a metal crystal, is

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Materials/elasticity_tensor

##Stress Materials
The base class for constitutive equations to compute a stress ($\mathbf{\sigma}$) is `ComputeStressBase`. The `ComputeStressBase` class defines the properties `stress` and `elastic_strain`. It is a pure virtual class, requiring all children to override the method `computeQpStress()`.

Two elastic constitutive models have been developed, one that assumes small strains [ComputeLinearElasticStress](/ComputeLinearElasticStress.md), and a second which assumes finite strains and rotations increments [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md) The input file syntax for these materials is

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Materials/stress

There are a number of other constitutive models that have been implemented to calculate more complex elasticity problems, plasticity, and creep.  An overview of these different materials is available on the [Stresses](tensor_mechanics/Stresses.md) page.

## Eigenstrain Materials

Eigenstrain is the term given to a strain which does not result directly from an applied force. The base class for eigenstrains is `ComputeEigenstrainBase`. It computes an eigenstrain, which is subtracted from the total strain in the Compute Strain classes.
\begin{equation}
\epsilon_{mechanical} = \epsilon_{total} - \epsilon_{eigen}
\end{equation}
[Chapter 3](http://onlinelibrary.wiley.com/doi/10.1002/9780470117835.ch3/pdf) of **Fundamentals of Micromechanics of Solids** describes the relationship between total, elastic, and eigen- strains and provides examples using thermal expansion and dislocations.

Eigenstrains are also referred to as residual strains, stress-free strains, or intrinsic strains; translated from German, [Eigen](http://dict.tu-chemnitz.de/deutsch-englisch/Eigen....html) means own or intrinsic in English.  The term eigenstrain was introduced by [T. Mura in 1982](http://link.springer.com/chapter/10.1007/978-94-011-9306-1_1):

> Eigenstrain is a generic name given to such nonelastic strains as thermal expansion, phase transformation, initial strains, plastic, misfit strains. Eigenstress is a generic name given to self-equilibrated internal stresses caused by one or several of these eigenstrains in bodies which are free from any other external force and surface constraint.  The eigenstress fields are created by the incompatibility of the eigenstrains.  This new English terminology was adapted from the German "Eigenspannungen and Eigenspannungsquellen," which is the title of H. Reissner's paper (1931) on residual stresses.

Thermal strains are a volumetric change resulting from a change in temperature of the material.  The change in strains can be either a simple linear function of thermal change, e.g. ($\mathbf{\epsilon}_T = \alpha \Delta T$) or a more complex function of temperature.
The thermal expansion class, [ComputeThermalExpansionEigenstrain](/ComputeThermalExpansionEigenstrain.md) computes the thermal strain  as a linear function of temperature.  The input file syntax is
!listing modules/tensor_mechanics/test/tests/thermal_expansion/constant_expansion_coeff.i block=Materials/thermal_expansion_strain

The eigenstrain material block name must also be added as an input parameter, `eigenstrain_names` to the strain material or TensorMechanics master action block. An example of the additional parameter in the TensorMechanics master action is shown below.
!listing modules/tensor_mechanics/test/tests/thermal_expansion/constant_expansion_coeff.i block=Modules/TensorMechanics

Other eigenstrains could be caused by defects such as over-sized or under-sized second phase particles. Such an eigenstrain material is [ComputeVariableEigenstrain](/ComputeVariableEigenstrain.md). This class computes a lattice mismatch due to a secondary phase, where the form of the tensor is defined by an input vector, and the scalar dependence on a phase variable is defined in another material. The input file syntax is

!listing modules/combined/test/tests/eigenstrain/inclusion.i block=Materials/eigenstrain

Note the `DerivativeParsedMaterial`,  which evaluates an expression given in the input file, and its automatically generated derivatives, at each quadrature point.
!listing modules/combined/test/tests/eigenstrain/inclusion.i block=Materials/var_dependence

## Extra Stress Materials

Extra stresses ($\mathbf{\sigma}_0$) can also be pulled into the residual calculation after the constitutive model calculation of the stress. The extra stress material property, `extra_stress` is defined in the `ComputeExtraStressBase` class and is added to the stress value.
\begin{equation}
  \sigma_{ij} = \sigma_{ij} + \sigma^{extra}_{ij}
\end{equation}

An extra stress may be a residual stress, such as in large civil engineering simulations.
An example of an extra stress material used in an input file is:
!listing modules/combined/test/tests/linear_elasticity/extra_stress.i block=Materials/const_stress
