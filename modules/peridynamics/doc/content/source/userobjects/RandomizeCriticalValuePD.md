# Randomize Critical Value UserObject

## Description

UserObject `RandomizeCriticalValuePD` is used to assign randomized values for the `critical_variable` auxiliary variable in fracture modeling. The generated values can be either uniform or follow normal distribution.

The same sampling should be applied to all the bonds, and usage of GeneralUserObject rather than AuxKernel can achieve this purpose by avoiding repetition of the same sampling procedure to partial bonds on each processor.

!syntax parameters /UserObjects/RandomizeCriticalValuePD

!syntax inputs /UserObjects/RandomizeCriticalValuePD

!syntax children /UserObjects/RandomizeCriticalValuePD
