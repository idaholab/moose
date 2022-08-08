# Scalar Material Damage

!syntax description /Materials/NonLocalDamage

## Description

`NonLocalDamage` is a model to define the effect of damage on the stress and stiffness in a continuum damage mechanics setting. It does not directly compute the stress, but must be used in conjunction with [ComputeDamageStress](/ComputeDamageStress.md).

This model is a scalar damage model in which the stress $\boldsymbol{\sigma}$ is computed as a function of the damage $d$, the original stiffness of the material $\mathbb{C}$ and the elastic strain $\boldsymbol{\varepsilon}$:

\begin{equation}
    \boldsymbol{\sigma} = (1 - d)\ \mathbb{C} : \boldsymbol{\varepsilon}
\end{equation}

The damage variable $d$ itself is computed as a nonlocal extension of an
external scalar damage model defined by the `local_damage_model`
and `average_UO` input parameters. Where `average_UO` is a RadialAverage user
object that defines the nonlocal averaging properties. The damage value is
delayed by a single `execte_on` (timestep or nonlinear).


!syntax parameters /Materials/NonLocalDamage

!syntax inputs /Materials/NonLocalDamage

!syntax children /Materials/NonLocalDamage

!bibtex bibliography
