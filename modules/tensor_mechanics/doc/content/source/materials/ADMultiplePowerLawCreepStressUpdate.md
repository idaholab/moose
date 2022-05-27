# ADMultiplePowerLawCreepStressUpdate

!syntax description /Materials/ADMultiplePowerLawCreepStressUpdate

## Description

Metals can exihibit distinctively different creep behavior depending on the stress
level. This object represents creep behavior with various power law expressions (following
logic analogous to [PowerLawCreepStressUpdate](PowerLawCreepStressUpdate.md)), 
and selects the right set of parameters for the von Mises stress value seen during
the return mapping. 

A set of von Mises stress intervals is required in the input. `stress_thresholds` is a 
vector of stresses defining a minimum and maximum modeled stress. In between pairs of
stress values, in strict order, the corresponding ordered power law parametes must be
provided, i.e. `coefficient`, `n_exponent`, `m_exponent`, and `activation_energy`.
 
This class relies on the implicit integration algorithm in
[!cite](dunne2005introduction) pg. 146 - 149.

`ADMultiplePowerLawCreepStressUpdate` must be run in conjunction with an inelastic
strain return mapping stress calculator such as
[ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md)

!syntax parameters /Materials/ADMultiplePowerLawCreepStressUpdate

!syntax inputs /Materials/ADMultiplePowerLawCreepStressUpdate

!syntax children /Materials/ADMultiplePowerLawCreepStressUpdate

!bibtex bibliography
