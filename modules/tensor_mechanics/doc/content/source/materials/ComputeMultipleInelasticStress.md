# Compute Multiple Inelastic Stress

!syntax description /Materials/ComputeMultipleInelasticStress

## Description

`ComputeMultipleInelasticStress` computes the stress, the consistent tangent
operator (or an approximation), and a decomposition of the strain
into elastic and inelastic components for a series of different inelastic
material models (e.g. creep and plasticity) which inherit from `StressUpdateBase`.
By default finite strains are assumed.
The elastic strain is calculated by subtracting the computed inelastic strain
increment tensor from the mechanical strain increment tensor.
\begin{equation}
  \label{cmis_elastic_strain_definition}
  \Delta \boldsymbol{\epsilon}^{el} = \Delta \boldsymbol{\epsilon}^{mech} - \Delta \boldsymbol{\epsilon}^{inel}
\end{equation}
Mechanical strain, $\epsilon^{mech}$, is considered to be the sum of the elastic
and inelastic (e.g. plastic and creep) strains.

!alert! note title=Default Finite Strain Formulation
This class uses the finite incremental strain formulation as a default. Users may
elect to use a small incremental strain formulation and set
+`perform_finite_strain_rotations = false`+ if the simulation will only ever use
small strains.
This class is not intended for use with a total small linearize strain formulation.
!alert-end!

`ComputeMultipleInelasticStress` is designed to be used in conjunction with a
separate model or set of models that computes the inelastic strain for a given
stress state. These inelastic models must derive from the `StressUpdateBase` class.
The Tensor Mechanics module contains a wide variety of such models, including

- [LinearViscoelasticStressUpdate](/LinearViscoelasticStressUpdate.md)
- `MultiParameterPlasticityStressUpdate`
- [RadialReturnStressUpdate](/RadialReturnStressUpdate.md)

!alert note title=Compatible Inelastic +`StressUpdate`+ Material Models
All of the inelastic material models that are compatible with
+`ComputeMultipleInelasicStress`+ follow the nomenclature convention of +`StressUpdate`+
as a suffix to the class name.

`ComputeMultipleInelasticStress` can accomodate as few as zero inelastic models (in which case the
algorithm from [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md)
is applied) to as many inelastic material models as is required by the physics.
If more than one inelastic material model is supplied to `ComputeMultipleInelasticStress`,
it is recommended that all of the inelastic models inherit from the same base class.


## Multiple Inelastic Models

!media tensor_mechanics/flowchart_ComputeMultipleInelasticStress.png
       id=fig:multiple_materials
       style=width:55%;margin-left:2%;float:right
       caption=The `ComputeMultipleInelasticStress` algorithm for calculating the
               strains and stresses for multiple inelastic material models.

The algorithm used to compute the stress for multiple inelastic models is shown
in [fig:multiple_materials].

When multiple inelastic models are given, `ComputeMultipleInelasticStress` iterates
over the specified inelastic models until the change in stress is within
a user-specified tolerance.

### Inner Iteration over Inelastic Models

The inner iteration over the multiple inelastic material models is shown in the
green components in [fig:multiple_materials].

When each inelastic model is evaluated, a trial stress is computed using the
current elastic strain, which is the total mechanical strain minus the current
summation of inelastic strain for all inelastic models. This trial stress can be
expressed as
\begin{equation}
  \label{eqn:trial_stress}
  \sigma^{tr}_i = \begin{cases}
                    C_{ijkl} \cdot \left( \epsilon^{el}_{old} + \Delta \epsilon^{el}_i \right) & \text{if $C_{ijkl}$ is isotropic}  \\
                    \sigma_{old} + C_{ijkl} \cdot \left( \Delta \epsilon^{el}_i \right) & \text{if $C_{ijkl}$ is anisotropic}
                  \end{cases}
\end{equation}
where $C_{ijkl}$ is the elasticity tensor for the material.

The $i^{th}$ inelastic material model, represented by the blue element
[fig:multiple_materials], is then called. The inelastic material model calculates
the inelastic strain increment necessary to produce an admissible stress, as a
function of the trial stress. The total inelastic strain increment is updated
for each model's contribution. The details of this calculation vary by model
and can include the effects of plasticity or creep.

The elastic and inelastic strain increments,
stress, and, optionally, the consistent tangent operator are returned to
`ComputeMultipleInelasticStress` from the $i^{th}$ inelastic material model.

### Outer Iteration over Stress Difference

After each inelastic model is called to compute an update to the stress tensor,
the minimum and maximum values of each component of the stress tensor, over the
course of those iterations, are stored to two tensors denoted as
$\boldsymbol{\sigma}_{max}$ and $\boldsymbol{\sigma}_{min}$, respectively. An
$L^2$ norm of the difference of these two tensors is then computed as
\begin{equation}
  \label{eqn:l2_norm_stress}
  L^2 \left(\Delta \boldsymbol{\sigma} \right) = \sqrt{\Delta \sigma_{ij} \Delta \sigma_{ij}}
    \text{, where } \Delta \boldsymbol{\sigma} = \boldsymbol{\sigma}_{max} - \boldsymbol{\sigma}_{min}
\end{equation}
The $L^2$ norm of the stress difference is compared to the absolute and relative
tolerances to determine if the solution from the combined inelastic material
models is converged
\begin{equation}
  \label{eqn:l2_norm_convergence}
  L^2 \left(\Delta \boldsymbol{\sigma} \right) < \text{absolute tolerance, or }
  \frac{L^2 \left(\Delta \boldsymbol{\sigma} \right)}{L^2 \left(\Delta \boldsymbol{\sigma}_o \right)} < \text{relative tolerance}
\end{equation}
where $L^2 \left(\Delta \sigma_o \right)$ is the $L^2$ norm from the very first
outer iteration over all of the inelastic material models.
The solution will not converge if the outer iteration loop, shown in the top half
of [fig:multiple_materials], exceeds the maximum number of iterations set by the
user.

### Finalize Strains and the Jacobian Multiplier

Once convergence on the stress is obtained, the calculation of the inelastic
strains is finalized by by applying a weighting factor, as shown in
[fig:multiple_materials]. This weighting factor has a default value of unity.

#### Jacobian Multiplier and the Consistent Tangent Operator

The Jacobian multiplier, which is used in the [StressDivergenceTensors](/StressDivergenceTensors.md)
kernel to condition the Jacobian calculation, must be calculated from the combination
of all the different inelastic material models. There are three options used to
calculate the combined Jacobian multiplier: Elastic, Partial, and Nonlinear, which
are set by the individual elastic material models.
\begin{equation}
  \label{eqn:combined_jacobian_mult}
  \boldsymbol{J}_m = \begin{cases}
                  \boldsymbol{C} & \text{Elastic option} \\
                  \boldsymbol{A}^{-1} \cdot \boldsymbol{C} \text{, where }
                      \boldsymbol{A} = \boldsymbol{I} + \sum_i \boldsymbol{H}^{cto}_i & \text{Partial option} \\
                  \prod_i \boldsymbol{H}^{cto}_i \cdot \boldsymbol{C}^{-1}  & \text{Nonlinear option}
                 \end{cases}
\end{equation}
where $\boldsymbol{J}_m$ is the Jacobian multiplier, $\boldsymbol{C}$ is the elasticity
tensor, $\boldsymbol{I}$ is the Rank-4 identity tensor, and $\boldsymbol{H}^{cto}$ is the
consistent tangent operator.

The consistent tangent operator, defined in [eqn:elastic_cto] provides the information
on how the stress changes with respect to changes in the displacement variables.
\begin{equation}
  \label{eqn:elastic_cto}
  \delta \sigma_{ij} = H_{ijkl} \delta \epsilon_{kl}
\end{equation}
where $\delta \epsilon_{kl}$ is an arbitrary change in the total strain
(which occurs because the displacements are changed) and $\delta \sigma_{ij}$
is the resulting change in the stress.
In a purely elastic situation $H_{ijkl} = C_{ijkl}$ (the elasticity tensor), but
the inelastic mapping of changes in the stress as a result of changes in the
displacement variables is more complicated.
In a plastic material model, the proposed values
of displacements for the current time step where used to calculate a trial
inadmissible stress, $\sigma_{trial}$, [eqn:trial_stress], that was brought back
to the yield surface through a radial return algorithm. A slight change in the
proposed displacement variables will produce a slightly different trial stress
and so on.
Other inelastic material models follow a similar pattern.

The user can chose to force all of the inelastic material models to use the elasticity
tensor as the consistent tangent operator by setting `tangent_operator = elastic`.
This setting will reduce the computational load of the inelastic material models
but may hamper the convergence of the simulation.
By default, the inelastic material models are allowed to compute the consistent
tangent operator implemented in each individual inelastic model with the
`tangent_operator = nonlinear` option.

#### Material Time Step Size Limitations

Prior to calculating the final strain values, the algorithm checks the size of
the current time step against any limitations on the size of the time step as
optionally defined by the inelastic material models.
As described in the Material Time Step Limiter section, the time step size
involves a post processor to ensure that the current time step size is reasonable
for each of the inelastic material models used in the simulation.

At the end of the algorithm, the final value of the elastic and inelastic
strain tensors are calculated as shown in the last element of [fig:multiple_materials].


## Single Inelastic Model

`ComputeMultipleInelasticStress` can also be used to calculate the inelastic
strain and the stress when only a single inelastic material model is provided.

!media tensor_mechanics/flowchart_ComputeMultipleInelasticStress-SingleModel.png
       id=fig:single_material
       style=width:40%;margin-right:2%;float:left
       caption=The optimized algorithm for calculating the strains and stress
               in the case when only a single inelastic material model is specified.

The algorithm, shown in [fig:single_material], used for a single inelastic material
model is an optimized version of the multiple materials algorithm.
With no need to iterate over multiple inelastic models, both the inner and outer
iterations from [fig:multiple_materials] are removed from the algorithm in [fig:single_material].

The initial elastic strain increment guess is assumed to be the initial mechanical
strain increment, and the trial stress for the single inelastic model is calculated
from that elastic strain increment as in [eqn:trial_stress].
These stress and strain values are passed directly to the inelastic material model.

The material model computes the admissible stress and strain states, as indicated
by the blue element in [fig:single_material]. An optional consistent tangent
operator matrix is also returned by the inelastic material model.
As in the multiple inelastic models algorithm, the user may force the use of the
Elastic option by setting `tangent_operator = elastic`.
By default, the inelastic material model is allowed to compute the consistent
tangent operator implemented in each individual inelastic model with the
`tangent_operator = nonlinear` option.

The consistent tangent operator is then used to find the Jacobian multiplier with
\begin{equation}
  \label{eqn:single_model_jacobian_mult}
  \boldsymbol{J}_m = \begin{cases}
                  \boldsymbol{C} & \text{Elastic option} \\
                  \left(\boldsymbol{I} + \boldsymbol{H}^{cto}\right)^{-1} \cdot \boldsymbol{C} & \text{Partial option} \\
                  \boldsymbol{H}^{cto}  & \text{Nonlinear option}
                 \end{cases}
\end{equation}
where $\boldsymbol{J}_m$ is the Jacobian multiplier, $\boldsymbol{C}$ is the elasticity
tensor, $\boldsymbol{I}$ is the Rank-4 identity tensor, and $\boldsymbol{H}^{cto}$ is the
consistent tangent operator, as discussed in the multiple inelastic material
models section.

The maximum size of the allowable time step is then optionally calculated by the
inelastic material model, as described in the section below on the Material Time
Step Limiter. At the conclusion of the algorithm, the value of the elastic and
inelastic strain states are updated as shown in [fig:single_material].

### Cycle Through One Inelastic Model per Time Step

`ComputeMultipleInelasticStress` also includes an option to run a series of inelastic
models in a rotating fashion such that only a single inelastic model is run on a
timestep. This option uses the same algorithm as in [fig:single_material] to determine
the strains and stress value based on the rotated single inelastic model.
A separate method is then employed to propagate the strain and stress values to
the other inelastic material models for storage as old material property values.

## Other Calculations Performed by `StressUpdate` Materials

The `ComputeMultipleInelasticStress` material relies on two helper calculations
to aid the simulation in converging.
These helper computations are defined within the specific inelastic models, and
only a brief overview is given here.
These methods are represented within the blue inelastic material model boxes in
[fig:multiple_materials] and [fig:single_material].
For specific details of the implementations, see the documentation pages for the
individual inelastic `StressUpdate` materials.

The first helper computation, the consistent tangent operator, is an optional
feature which is implemented for only certain inelastic
stress material models, and the material time step limiter is implemented in the
models which use the [Radial Return Stress Update](/RadialReturnStressUpdate.md)
algorithm.

### Consistent Tangent Operator

The consistent tangent operator is used to improve the convergence of mechanics
problems (see a reference such as [!cite](simo1985cto) for an introduction to
consistent tangent operators).
The Jacobian matrix, [eqn:combined_jacobian_mult] and [eqn:single_model_jacobian_mult],
is used to capture how the change in the residual calculation changes with respect
to changes in the displacement variables.
To calculate the Jacobian, MOOSE relies on knowing how the stress changes with
respect to changes in the displacement variables.

Because the change of the stress with respect to the change in displacements is
material specific, the value of the consistent tangent operator
is computed in each inelastic material model. By default the consistent tangent
operator is set equal to the elasticity tensor (the option Elastic in
[eqn:combined_jacobian_mult] and [eqn:single_model_jacobian_mult]).
Inelastic material models which use either the Partial or Nonlinear options in
[eqn:combined_jacobian_mult] or [eqn:single_model_jacobian_mult] define a material
specific consistent tangent operator.

Generally Partial consistent tangent operators should be implemented for
non-yielding materials (e.g. volumetric swelling) and Full consistent tangent
operators should be implemented for yielding material models (e.g. plasticity).

### Include Damage Model

Optionally, the effect of damage on the stress calculation can be included in
the model. Another material that defines the evolution of damage should be
coupled using parameter `damage_model`. Here, first the inelastic strains and
corresponding effective stresses are calculated based on the undamaged
properties. Afterwards, the damage index is applied on the effective stress to
calculate the damaged stress. This captures the effect of damage in a material
undergoing creep or plastic deformation.

### Material Time Step Limiter

In some cases, particularly in creep, limits on the time step are required by
the material model formulation. Each inelastic material model is responsible for
calculating the maximum time step allowable for that material model. The
[MaterialTimeStepPostprocessor](/MaterialTimeStepPostprocessor.md) finds the
minimum time step size limits from the entire simulation domain. The
postprocessor then interfaces with the
[IterationAdaptiveDT](/IterationAdaptiveDT.md) to restrict the time step size
based on the limit calculated in the previous time step. When the damage model
is included, the timestep is limited by the minimum timestep between the
inelastic models and the damage model.


## Example Input Files

The input settings for multiple inelastic material models and a single inelastic
model are similar, and examples of both are shown below.

### Multiple Inelastic Models

For multiple inelastic models, all of the inelastic material
model names must be listed as arguments to the `inelastic_models` parameter.
The inelastic material blocks must also be present.

!listing modules/tensor_mechanics/test/tests/combined_creep_plasticity/combined_creep_plasticity.i block=Materials

### Single Inelastic Model

For a single inelastic material model the input syntax is simply condensed

!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i block=Materials/stress

and only a single inelastic material model is included in the input. This example
includes the `max_inelastic_increment` parameter which is used to limit the time
step size.

!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i block=Materials/isoplas

!syntax parameters /Materials/ComputeMultipleInelasticStress

!syntax inputs /Materials/ComputeMultipleInelasticStress

!syntax children /Materials/ComputeMultipleInelasticStress

!bibtex bibliography
