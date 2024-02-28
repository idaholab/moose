# Compute Damage Stress

!syntax description /Materials/ComputeDamageStress

## Description

`ComputeDamageStress` computes the stress for damaged elastic material in conjunction with another material that defines the evolution of damage, which must be derived from `DamageBase`. The only source of material nonlinearity in this model is the damage model, as there is no provision for including effects such as creep or plasticity.

See [ScalarMaterialDamage](/ScalarMaterialDamage.md) for a simple example of a damage material that can be used in conjunction with this model to define the effect of damage. The `ScalarMaterialDamage` model simply uses another scalar material property to define the evolution of the damage index.

!syntax parameters /Materials/ComputeDamageStress

!syntax inputs /Materials/ComputeDamageStress

!syntax children /Materials/ComputeDamageStress
