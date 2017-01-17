#TensorMechanics Module

* [Strains](tensor_mechanics/Strains.md)
* [Stresses](tensor_mechanics/Stresses.md)
* [Stress Divergence](tensor_mechanics/StressDivergence.md)
* [Visualizing Tensors](tensor_mechanics/VisualizingTensors.md)
* [Tensor Classes](tensor_mechanics/TensorClasses.md)
* [Convergence Criteria](tensor_mechanics/Convergence.md)

The MOOSE tensor mechanics module is a library for simplifying the implementation of simulation tools that solve mechanics problems. It provides a simple approach for implementing even advanced mechanics models.

The tensor mechanics module provides a powerful system for solving solid mechanics problems using a simple to use syntax based on tensor forms. This approach allows the tensor equations to be implemented clearly and concisely.

##Mathematical Introduction

A material varies from its rest shape due to stress. This departure from the rest shape is called deformation or displacement, and the proportion of deformation to original size is called strain. To determine the deformed shape and the stress, a governing equation is solved to determine the displacement vector $\mathbf{u}$.

The strong form of the governing equation on the domain $\Omega$ and boundary $\Gamma=\Gamma_{\mathit{\iota_i}}\cup\Gamma_{\mathit{g_i}}$
can be stated as follows:
$$
\begin{eqnarray}
\nabla \cdot (\boldsymbol{\sigma} + \boldsymbol{\sigma}_0) + \mathbf{b} &=& \mathbf{0} \;\mathrm{in}\;\Omega \\
\mathbf{u} &=& \mathbf{g}\;\mathrm{in}\;\Gamma_{ \mathbf{g}} \\
\boldsymbol{\sigma} \cdot \mathbf{n}&=&\boldsymbol{\iota}\;\mathrm{in}\;\Gamma_{ \boldsymbol{\iota}}
\end{eqnarray}
$$
where $\boldsymbol{\sigma}$  is the Cauchy stress tensor, $\boldsymbol{\sigma}_0$ is an additional source of stress (such as pore pressure), $\mathbf{u}$ is the displacement vector, $\mathbf{b}$ is the body force, $\mathbf{n}$ is the unit normal to the boundary, $\mathbf{g}$ is the prescribed displacement on the boundary and $\boldsymbol{\iota}$ is the prescribed traction on the boundary. The weak form of the residual equation is expressed as:
$$
\begin{eqnarray}
  \mathbb{R} = \left( \boldsymbol{\sigma} + \boldsymbol{\sigma}_0), \nabla \phi_m \right) - \left< \boldsymbol{\iota}, \phi_m \right> - \left( \mathbf{b}, \phi_m \right)  = \mathbf{0},
\end{eqnarray}
$$
where $(\cdot)$ and $\left< \cdot \right>$ represent volume and boundary integrals, respectively. The solution of the residual equation with Newton's method requires the Jacobian of the residual equation, which can be expressed as (ignoring boundary terms)
$$
\begin{eqnarray}
  \mathbb{J} = \left( \frac{\partial \boldsymbol{\sigma}}{\partial \nabla \mathbf{u}} , \nabla \phi_m \right),
\end{eqnarray}
$$
assuming $\boldsymbol{\sigma}_0$ is independent of the strain.

The material stress response is described by the constitutive model, where the stress is determined as a function of the strain, i.e. $\tilde{\boldsymbol{\sigma}}( \boldsymbol{\epsilon} - \boldsymbol{\epsilon}_0)$, where $\boldsymbol{\epsilon}$ is the strain and $\boldsymbol{\epsilon}_0$ is a stress free strain. For example, in linear elasticity (only valid for small strains), the material response is linear, i.e.
$\boldsymbol{\sigma} = \boldsymbol{\mathcal{C}}(\boldsymbol{\epsilon} - \boldsymbol{\epsilon}_0)$.

The tensor mechanics system can handle linear elasticity and finite strain mechanics, including elasticity, plasticity, creep, and damage.

##Using TensorMechanics Materials

The tensor mechanics materials use a modular system where the main tensors used in the residual equation are defined in individual material classes in MOOSE. The plug-and-play approach used in the Tensor Mechanics module requires at least three separate classes to fully describe a material model.

!!! info "Three Tensors Are Required for a Mechanics Problem"
    The three tensors that must be defined for any mechanics problem are the the strain $\boldsymbol{\epsilon}$ or strain increment, elasticity tensor $\boldsymbol{\mathcal{C}}$, and the stress $\boldsymbol{\sigma}$. Optional tensors include stress-free strain (also known as an eigenstrain) $\boldsymbol{\epsilon}_0$ and additional stress $\boldsymbol{\sigma}_0$.

!image docs/media/tensor_mechanics-IntroPlugNPlay.png width=800 float=right caption=Figure 1: Tensors required to fully describe a mechanics material.


At times, a user may need to define multiple mechanics properties over a single block. For this reason, all material properties can be prepended by a name defined by the input parameter `base_name`.

###Strain or Strain increment
The base material class to create strains ($\boldsymbol{\epsilon}$) or strain increments is ComputeStrainBase](/ComputeStrainBase.md). `ComputeStrainBase` is a pure virtual class, requiring that all children override the `computeQpProperties()` method.

For all strain this class defines the property `total_strain`.  For incremental strains, both finite and small, the compute strain base class defines the properties
`strain_rate`, `strain_increment`, `rotation_increment`, and `deformation_gradient`. A discussion of the different types of strain formulations is available on the [Strains](tensor_mechanics/Strains.md) page.

For small strains, use [ComputeSmallStrain](/ComputeSmallStrain.md) in which $\boldsymbol{\epsilon} = (\nabla \mathbf{u} + \nabla \mathbf{u}^T)/2$. For finite strains, use [ComputeFiniteStrain](/ComputeFiniteStrain.md) in which an incremental form is employed such that the strain_increment and rotation_increment are calculated. The input file syntax for a finite incremental strain material is

!text modules/tensor_mechanics/tests/finite_strain_elastic/finite_strain_elastic_new_test.i start=strain end=stress overflow-y=scroll max-height=200px


###Elasticity Tensor

The primary class for creating elasticity tensors ($\boldsymbol{\mathcal{C_{ijkl}}}$) is [ComputeElasticityTensor](/ComputeElasticityTensor.md). This class defines the property
`_elasticity_tensor`. Given the elastic constants required for the applicable symmetry, such as `symmetric9`, this material calculates the elasticity tensor. If you wish to rotate the elasticity tensor, constant Euler angles can be provided. The elasticity tensor can also be scaled with a function, if desired. The input file syntax to create an elasticity tensor is

`ComputeElasticityTensor` also serves as a base class for specialized elasticity tensors, including:

* An elasticity tensor for crystal plasticity, [ComputeElasticityTensorCP](/ComputeElasticityTensorCP.md),
* A Cosserat elasticity tensor [ComputeLayeredCosseratElasticityTensor](/ComputeLayeredCosseratElasticityTensor.md),
* An isotropic elasticity tensor [ComputeIsotropicElasticityTensor](/ComputeIsotropicElasticityTensor.md).

The input file syntax for the isotropic elasticity tensor is

!text modules/tensor_mechanics/tutorials/basics/part_1.1.i start=elasticity_tensor end=strain overflow-y=scroll max-height=200px

and for an orthotropic material, such as a metal crystal, is

!text modules/tensor_mechanics/tests/finite_strain_elastic/finite_strain_elastic_new_test.i start=elasticity_tensor end=strain overflow-y=scroll max-height=300px

###Stress
The base class for constitutive equations to compute a stress ($\boldsymbol{\sigma}$) is `ComputeStressBase`. The `ComputeStressBase` class defines the properties `stress` and `elastic_strain`. It is a pure virtual class, requiring all children to override the method `computeQpStress()`.

Two elastic constitutive models have been developed, one that assumes small strains [ComputeLinearElasticStress](/ComputeLinearElasticStress.md), and a second which assumes finite strains and rotations increments [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md) The input file syntax for these materials is

!input modules/tensor_mechanics/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=stress overflow-y=scroll max-height=500px

There are a number of other constitutive models that have been implemented to calculate more complex elasticity problems, plasticity, and creep.  An overview of these different materials is available on the [Stresses](tensor_mechanics/Stresses.md) page.

### Stress-Free Strains (Eigenstrains)

Eigenstrain is the term given to a strain which does not result directly from an applied force. Eigenstrains are also referred to as residual strains, stress-free strains, or intrinsic strains; translated from German, [Eigen](http://dict.tu-chemnitz.de/deutsch-englisch/Eigen....html) means own or intrinsic in English.  The term eigenstrain was introduced by [T. Mura in 1982](http://link.springer.com/chapter/10.1007/978-94-011-9306-1_1):

> Eigenstrain is a generic name given to such nonelastic strains as thermal expansion, phase transformation, initial strains, plastic, misfit strains. Eigenstress is a generic name given to self-equilibrated internal stresses caused by one or several of these eigenstrains in bodies which are free from any other external force and surface constraint.  The eigenstress fields are created by the incompatibility of the eigenstrains.  This new English terminology was adapted from the German "Eigenspannungen and Eigenspannungsquellen," which is the title of H. Reissner's paper (1931) on residual stresses.

The base class for eigenstrains is `ComputeEigenstrainBase`. It computes an eigenstrain, which is subtracted from the total strain in the Compute Strain classes. Eigenstrains are calculated separately from `mechanical_strains`: elastic, plastic, and creep strains are considered mechanical strains in Tensor Mechanics.

$$
\epsilon_{mechanical} = \epsilon_{total} - \epsilon_{eigen}
$$

The mechanical strain is passed to the `Compute*Stress` methods to calculate the stress for the current iteration.  [Chapter 3](http://onlinelibrary.wiley.com/doi/10.1002/9780470117835.ch3/pdf) of **Fundamentals of Micromechanics of Solids** describes the relationship between total, elastic, and eigen strains and provides examples with thermal expansion and dislocations.

Thermal strains are a volumetric change resulting from a change in temperature of the material.  The change in strains can be either a simple linear function of thermal change, e.g. ($\boldsymbol{\epsilon}_T = \alpha \Delta T$) or a more complex function of temperature.   Besides thermal expansion, some models employ other stress-free strains ($\boldsymbol{\epsilon}_0$) to provide inherit strains in the material.

The thermal expansion class, [ComputeThermalExpansionEigenstrain](/ComputeThermalExpansionEigenStrain.md) computes the thermal strains for both small total strains and for incremental strains as a linear function of temperature.  The input file syntax is

!input modules/tensor_mechanics/tests/thermal_expansion/constant_expansion_coeff.i block=thermal_expansion_strain overflow-y=scroll max-height=300px

Other eigenstrains could be caused by defects such as over-sized or under-sized second phase particles. Another stress-free strain material that has been implemented is [ComputeVariableEigenstrain](/ComputeVariableEigenstrain.md). This class computes a lattice mismatch due to a secondary phase, where the form of the tensor is defined by an input vector, and the scalar dependence on a phase variable is defined in another material. The input file syntax is

!input modules/combined/tests/eigenstrain/inclusion.i block=var_dependence overflow-y=scroll max-height=400px

!input modules/combined/tests/eigenstrain/inclusion.i block=eigen_strain overflow-y=scroll max-height=400px

Note the `DerivativeParsedMaterial`,  which evaluates an expression given in the input file, and its automatically generated derivatives, at each quadrature point.

### Extra Stresses

Extra stresses ($\boldsymbol{\sigma}_0$) can also be pulled into the residual calculation after the constitutive model calculation of the stress. The extra stress is added to the stress value
```
  _stress[_qp] += _extra_stress[_qp];
```

 Though no base class has been created for this, it would be straight forward to do so and it would need to define the property `extra_stress`.
