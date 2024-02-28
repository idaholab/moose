# NonlocalDamage

!syntax description /Materials/NonlocalDamage

## Description

`NonlocalDamage` is a model to define the effect of damage on the stress and stiffness in a continuum damage mechanics setting. It does not directly compute the stress, but must be used in conjunction with [ComputeDamageStress](/ComputeDamageStress.md).


The damage variable $d$ itself is computed as a nonlocal extension of an
external scalar damage model defined by the `local_damage_model`
and `average_UO` input parameters. The `average_UO` is a [RadialAverage](/RadialAverage) user
object that defines the nonlocal averaging properties. The damage value is
delayed by a single `execte_on` (timestep or nonlinear).


!syntax parameters /Materials/NonlocalDamage

!syntax inputs /Materials/NonlocalDamage

!syntax children /Materials/NonlocalDamage

!bibtex bibliography
