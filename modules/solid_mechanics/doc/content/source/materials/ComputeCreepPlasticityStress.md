# Compute Creep and Plasticity Inelastic Stress

!syntax description /Materials/ComputeCreepPlasticityStress

## Description

`ComputeCreepPlasticityStress` computes the stress, the consistent tangent
operator (or an approximation), and a decomposition of the strain
into elastic and inelastic components for a pair inelastic
material models, namely creep and plasticity.
By default finite strains are assumed.

The elastic strain is calculated by subtracting the computed inelastic strain
increment tensor from the mechanical strain increment tensor.
\begin{equation}
  \label{cmis_elastic_strain_definition}
  \Delta \boldsymbol{\epsilon}^{el} = \Delta \boldsymbol{\epsilon}^{mech} - \Delta \boldsymbol{\epsilon}^{inel}
\end{equation}
Mechanical strain, $\epsilon^{mech}$, is considered to be the sum of the elastic
and inelastic (creep and plastic) strains.

!alert! note title=Default Finite Strain Formulation
This class uses the finite incremental strain formulation as a default. Users may
elect to use a small incremental strain formulation and set
+`perform_finite_strain_rotations = false`+ if the simulation will only ever use
small strains.
This class is not intended for use with a total small linearize strain formulation.
!alert-end!

`ComputeCreepPlasticityStress` requires one creep model and one
plasticity model.  These need to be "stress update" models that derive from the
following base classes for creep and plasticity:

- [PowerLawCreepStressUpdate](PowerLawCreepStressUpdate.md)
- [IsotropicPlasticityStressUpdate](IsotropicPlasticityStressUpdate.md)

## Combined Newton Iteration

The power law creep equation is
\begin{equation}
  \dot{p}_c=A\sigma^n_e=A(\sigma^{tr}_e-3G(\Delta p_c + \Delta p_p))^n
\end{equation}
where $\dot{p}_c$ is the creep inelastic strain rate, $A$ is a prefactor that does
not depend on stress, $\sigma_e$ is the effective stress, $\sigma^{tr}_e$ is the
trial effective stress, $G$ is the shear modulus, $\Delta p_c$ is the creep inelastic
strain increment, $\Delta p_p$ is the plastic inelastic strain increment, and $n$ is
the exponent.  A residual $f_c$ is formed as
\begin{equation}
f_c = \dot{p}_c \Delta t - \Delta p_c = 0
\end{equation}

The isotropic plasticity residual $f_p$ is
\begin{equation}
f_p = \sigma^{tr}_e - 3G(\Delta p_c + \Delta p_p)-r-\sigma_y=0
\end{equation}
where $r$ is the hardening value and $\sigma_y$ is the yield stress.

A Taylor expansion of $f(x)$ with two terms is
\begin{equation}
f(x) \approx f(a) + f'(a)(x-a)
\end{equation}

By setting $f_c$ and $f_p$ to zero and using this two-term Taylor expansion, we obtain
\begin{equation}
\begin{bmatrix}
\frac{\partial f_c}{\partial\Delta p_{c0}} & \frac{\partial f_c}{\partial\Delta p_{p0}} \\
\frac{\partial f_p}{\partial\Delta p_{c0}} & \frac{\partial f_p}{\partial\Delta p_{p0}}
\end{bmatrix}
\begin{bmatrix}
\Delta p_c - \Delta p_{c0} \\
\Delta p_p - \Delta p_{p0}
\end{bmatrix} =
\begin{bmatrix}
-f_c \\
-f_p
\end{bmatrix}
\end{equation}

where the subscript $0$ represents the current value.  This leads to

\begin{equation}
\begin{bmatrix}
\Delta p_c \\
\Delta p_p
\end{bmatrix} =
\begin{bmatrix}
\Delta p_{c0} \\
\Delta p_{p0}
\end{bmatrix} -
\begin{bmatrix}
\frac{\partial f_c}{\partial\Delta p_{c0}} & \frac{\partial f_c}{\partial\Delta p_{p0}} \\
\frac{\partial f_p}{\partial\Delta p_{c0}} & \frac{\partial f_p}{\partial\Delta p_{p0}}
\end{bmatrix}^{-1}
\begin{bmatrix}
f_c \\
f_p
\end{bmatrix}
\end{equation}

The diagonal terms in the matrix are the derivative of the creep residual with respect to the inelastic creep increment and the derivative of the plasticity residual with respect to the inelastic plastic strain increment.  These are available from the two inelastic models.

#### Jacobian Multiplier and the Consistent Tangent Operator

The Jacobian multiplier, which is used in the [StressDivergenceTensors](/StressDivergenceTensors.md)
kernel to condition the Jacobian calculation, must be calculated from the combination
of the two inelastic material models. There are three options used to
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
of displacements for the current time step were used to calculate a trial
inadmissible stress, $\sigma_{trial}=C_{ijkl} ( \epsilon^{el}_{old} + \Delta \epsilon^{el}_i )$,
that was brought back
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
As described in the [#limiter] section, the time step size
involves a post processor to ensure that the current time step size is reasonable
for each of the inelastic material models used in the simulation.

At the end of the algorithm, the final value of the elastic and inelastic
strain tensors are calculated by adding the increments to the old values.


## Other Calculations Performed by `StressUpdate` Materials

The `ComputeCreepPlasticityStress` material relies on two helper calculations
to aid the simulation in converging.
These helper computations are defined within the specific inelastic models, and
only a brief overview is given here.
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
The Jacobian matrix, [eqn:combined_jacobian_mult],
is used to capture how the change in the residual calculation changes with respect
to changes in the displacement variables.
To calculate the Jacobian, MOOSE relies on knowing how the stress changes with
respect to changes in the displacement variables.

Because the change of the stress with respect to the change in displacements is
material specific, the value of the consistent tangent operator
is computed in each inelastic material model. By default the consistent tangent
operator is set equal to the elasticity tensor (the option Elastic in
[eqn:combined_jacobian_mult]).
Inelastic material models which use either the Partial or Nonlinear options in
[eqn:combined_jacobian_mult] define a material
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

### Material Time Step Limiter id=limiter

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


## Example Input File

!listing modules/tensor_mechanics/test/tests/combined_creep_plasticity/creepWithPlasticity.i block=Materials


!syntax parameters /Materials/ComputeCreepPlasticityStress

!syntax inputs /Materials/ComputeCreepPlasticityStress

!syntax children /Materials/ComputeCreepPlasticityStress

!bibtex bibliography
