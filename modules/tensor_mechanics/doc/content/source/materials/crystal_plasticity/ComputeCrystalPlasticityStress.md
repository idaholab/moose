# ComputeCrystalPlasticityStress

!syntax description /Materials/ComputeCrystalPlasticityStress

## Description

!alert warning
Documentation under development

`ComputeCrystalPlasticityStress` calls the specified crystal plasticity
constitutive model class and stores the Cauchy stress calculated by the crystal
plasticity model.

`ComputeCrystalPlasticityStress` is designed to be used in conjunction with a
crystal plasticity model the inelastic strain. These models must derive from the `CrystalPlasticityUpdate` class.

## Example Input File

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/stress

!syntax parameters /Materials/ComputeCrystalPlasticityStress

!syntax inputs /Materials/ComputeCrystalPlasticityStress

!syntax children /Materials/ComputeCrystalPlasticityStress

!bibtex bibliography
