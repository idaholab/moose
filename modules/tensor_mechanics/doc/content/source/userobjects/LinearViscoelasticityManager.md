# Linear Viscoelasticity Manager

!syntax description /UserObjects/LinearViscoelasticityManager

## Description

This updates the internal time-stepping scheme for linear viscoelastic materials (such as a [GeneralizedKelvinVoigtModel](/GeneralizedKelvinVoigtModel.md) or a [GeneralizedMaxwellModel](/GeneralizedMaxwellModel.md) material) at the beginning of each time step. Including a LinearViscoelasticStressUpdate in the simulation is required for the linear viscoelastic strains to be updated properly.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/visco/visco_finite_strain.i block=UserObjects/update

!syntax parameters /UserObjects/LinearViscoelasticityManager

!syntax inputs /UserObjects/LinearViscoelasticityManager

!syntax children /UserObjects/LinearViscoelasticityManager
