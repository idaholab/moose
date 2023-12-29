# Temperature Dependent Hardening Stress Update

!syntax description /Materials/TemperatureDependentHardeningStressUpdate

## Description

!include modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md

## Isotropic Plasticity

`TemperatureDependentHardeningStressUpdate` is formulated in the same manner as
[IsotropicPlasticityStressUpdate](/IsotropicPlasticityStressUpdate.md) such that
the effective plastic strain increment has the form
\begin{equation}
  \label{eqn:effective_strain_incr}
  d \Delta p = \frac{\sigma^{trial}_{effective} - 3 G \Delta p - r - \sigma_y}{3G + h}
\end{equation}
where $G$ is the isotropic shear modulus, $\sigma^{trial}_{effective}$ is the
scalar von Mises trial stress, $\sigma_y$ is the yield stress, $r$ is the
hardening function, and $h$ is the derivative of the hardening function with
respect to the trial stress.

### Temperature Dependent Hardening

The temperature dependence in [eqn:effective_strain_incr] is captured in the
hardening function and the hardening function derivative.
\begin{equation}
  \label{eqn:hardening_function}
  \begin{aligned}
    r & = \left(1 - T_{hf} \right) \cdot \mathit{f} \left( i_{low}, \Delta p \right)
          + T_{hf} \cdot \mathit{f} \left( i_{upper}, \Delta p \right) - \sigma_y \\
    h & = \left(1 - T_{hf} \right) \cdot \mathit{f} \left( i_{low}, \Delta p_{old} \right)
          + T_{hf} \cdot \mathit{f} \left( i_{upper}, \Delta p_{old} \right)
  \end{aligned}
\end{equation}
where $T_{hf}$ is the relative temperature fraction within the lower and upper
temperature bounds of the current piecewise function section, $i_{low}$ is the
index of the lower temperature bound and its corresponding hardening function, and
$i_{upper}$ is the upper temperature bound and hardening function index, and
$\Delta p$ and $\Delta p_{old}$ are the current and old effective strain
increments, respectively.
The relative temperature fraction is defined as
\begin{equation}
  \label{eqn:temperature_fraction}
  T_{hf} = \frac{T - T(i_{low})}{T(i_{upper}) - T(i_{low})}
\end{equation}
where $T$ is the current temperature, and $i_{low}$ and $i_{high}$ are the same
indices as defined for [eqn:hardening_function].
The $T_{hf}$ value is used in [eqn:hardening_function] to interpolate the
hardening values.

This class, `TemperatureDependentHardeningStressUpdate`, calculates an effective
trial stress, an effective scalar plastic strain increment, and the derivative
of the scalar effective plastic strain increment; these values are passed to the
[RadialReturnStressUpdate](/RadialReturnStressUpdate.md) to compute the radial
return stress increment.  The plastic strain as a stateful material property.


## Example Input File

!listing modules/tensor_mechanics/test/tests/temperature_dependent_hardening/temp_dep_hardening.i block=Materials/temp_dep_hardening

where the arguments for the `hardening_functions` parameter are defined in the
`Functions` block of the input file:

!listing modules/tensor_mechanics/test/tests/temperature_dependent_hardening/temp_dep_hardening.i block=Functions/hf1

!listing modules/tensor_mechanics/test/tests/temperature_dependent_hardening/temp_dep_hardening.i block=Functions/hf2

`TemperatureDependentHardeningStressUpdate` must be run in conjunction with the
inelastic strain return mapping stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/temperature_dependent_hardening/temp_dep_hardening.i block=Materials/radial_return_stress


!syntax parameters /Materials/TemperatureDependentHardeningStressUpdate

!syntax inputs /Materials/TemperatureDependentHardeningStressUpdate

!syntax children /Materials/TemperatureDependentHardeningStressUpdate
