# Scalar Material Damage

!syntax description /Materials/ScalarMaterialDamage

## Description

`ScalarMaterialDamage` is a model to define the effect of damage on the stress and stiffness in a continuum damage mechanics setting. It does not directly compute the stress, but must be used in conjunction with [ComputeDamageStress](/ComputeDamageStress.md).

This model is a scalar damage model that does not compute the evolution of damage by itself, but relies on another material model to supply a scalar damage index, which can vary between 0 (undamaged) and 1 (fully damaged). This model is mostly intended for use in testing the continuum damage mecahnics system, but could be used for modeling physical behavior if that were included in the model supplying the damage index.

!syntax parameters /Materials/ScalarMaterialDamage

!syntax inputs /Materials/ScalarMaterialDamage

!syntax children /Materials/ScalarMaterialDamage

!bibtex bibliography
