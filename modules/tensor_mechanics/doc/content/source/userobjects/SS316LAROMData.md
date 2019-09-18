# AD SS316 LAROM Data

!syntax description /UserObjects/SS316LAROMData

## Description

`SS316LAROMData` contains the relevant Los Alamos Reduced Order Model (ROM) data to compute the
creep behavior of stainless steel 316H.

`SS316LAROMData` must be run in conjunction with [ADLAROMCreepStressUpdate](ADLAROMCreepStressUpdate.md)

While `ADLAROMCreepStressUpdate` contains the necessary algorithms contained to evaluate the ROM,
the actual data that comprises the ROM is contained in a separate UserObject that inherets from
`LAROMData`. The data contained in `LAROMData` is the maximum and minimum values used to calibrate
the ROM, the transform used to prepare the input for the ROM, and the Legendre Polyonimal
coefficients.

`ADLAROMCreepStressUpdate` must be run in conjunction with an inelastic strain return mapping stress
calculator such as [ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md).

!listing modules/tensor_mechanics/test/tests/rom_stress_update/2drz.i block=Materials

In addition, the material specific `LAROMData` user object must also be coupled:

!listing modules/tensor_mechanics/test/tests/rom_stress_update/2drz.i block=UserObjects

!syntax parameters /UserObjects/SS316LAROMData

!syntax inputs /UserObjects/SS316LAROMData

!syntax children /UserObjects/SS316LAROMData

!bibtex bibliography
