# Power Law Creep Stress Update

!syntax description /Materials/PowerLawCreepStressUpdate

## Description

!include modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md

The increment of inelastic strain is computed from the creep rate in this class.

\begin{equation}
  \label{eq:power_law_creep}
  \dot{\epsilon} = \left( \sigma^{trial}_{effective} - 3 G \Delta p \right)^n exp \left( \frac{-Q}{RT} \right) \left(t - t_o \right)^m
\end{equation}

where $\sigma^{trial}_{effective}$ is the scalar von Mises trial stress, $G$ is
the isotropic shear modulus, $Q$ is the activation energy, $R$ is the universal
gas constant, $T$ is the temperature, $t$ and $t_o$ are the current and initial
times, respectively, and $n$ and $m$ are exponent values.

This class calculates an effective trial stress, an effective creep strain rate
increment and the derivative of the creep strain rate, and an effective scalar
inelastic strain increment; these values are passed to the
[ADRadialReturnStressUpdate](/ADRadialReturnStressUpdate.md) to compute the radial
return stress increment. This isotropic plasticity class also computes the
plastic strain as a stateful material property.

This class is based on the implicit integration algorithm in
[!cite](dunne2005introduction) pg. 146 - 149.

`PowerLawCreepStressUpdate` must be run in conjunction with an inelastic
strain return mapping stress calculator such as
[ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md)

!syntax parameters /Materials/PowerLawCreepStressUpdate

!syntax inputs /Materials/PowerLawCreepStressUpdate

!syntax children /Materials/PowerLawCreepStressUpdate

!bibtex bibliography
