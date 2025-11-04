# Composite Power Law Creep Stress Update

!syntax description /Materials/CompositePowerLawCreepStressUpdate

## Description

!include modules/solid_mechanics/common/supplementalRadialReturnStressUpdate.md

!include source/materials/PowerLawCreepStressUpdate.md

The increment of inelastic strain is computed from the creep rate in this class.

\begin{equation}
  \label{eq:composite_power_law_creep}
  \dot{\epsilon} = \sum_{i=1}^N h_i \left[ A_i \left( \sigma^{trial}_{effective} - 3 G \Delta p \right)^{n_i} \exp \left( \frac{-Q_i}{RT} \right) \right] \left(t - t_o \right)^m
\end{equation}

where subscript $i$ denotes phase specific material property, $h_i$ is the phase interpolation function, $A_i$ is the power law creep coefficient, also known as Dorn's Constant, $\sigma^{trial}_{effective}$ is the scalar von Mises trial stress, $G$ is
the isotropic shear modulus, $Q$ is the activation energy, $R$ is the universal
gas constant, $T$ is the temperature, $t$ and $t_o$ are the current and initial
times, respectively, and $n$ and $m$ are exponent values.

This class calculates an effective trial stress, an effective creep strain rate
increment and the derivative of the creep strain rate, and an effective scalar
inelastic strain increment based on different contributions from different phases; these values are passed to the
[ADRadialReturnStressUpdate](/ADRadialReturnStressUpdate.md) to compute the radial
return stress increment. This isotropic plasticity class also computes the
plastic strain as a stateful material property.

This class is based on the implicit integration algorithm in
[!cite](dunne2005introduction) pg. 146 - 149.

`CompositePowerLawCreepStressUpdate` must be run in conjunction with an inelastic
strain return mapping stress calculator such as
[ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md)

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/power_law_creep/composite_power_law_creep.i block=Materials/power_law_creep

!syntax parameters /Materials/CompositePowerLawCreepStressUpdate

!syntax inputs /Materials/CompositePowerLawCreepStressUpdate

!syntax children /Materials/CompositePowerLawCreepStressUpdate

!bibtex bibliography
