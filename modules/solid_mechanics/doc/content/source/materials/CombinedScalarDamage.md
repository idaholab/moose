# CombinedScalarDamage

!syntax description /Materials/CombinedScalarDamage

## Description

`CombinedScalarDamage` is a model to define the effect of damage on the stress and stiffness in a continuum damage mechanics setting. It does not directly compute the stress, but must be used in conjunction with [ComputeDamageStress](/ComputeDamageStress.md).

This model is a scalar damage model in which the stress $\boldsymbol{\sigma}$ is computed as a function of the damage $d$, the original stiffness of the material $\mathbb{C}$ and the elastic strain $\boldsymbol{\varepsilon}$:

\begin{equation}
    \boldsymbol{\sigma} = (1 - d)\ \mathbb{C} : \boldsymbol{\varepsilon}
\end{equation}

The damage variable $d$ itself is computed as a combination of a series of external scalar damage models defined by the `damage_models` input parameters.
Two combination types are possible: `Maximum` (default) and `Product`:

\begin{eqnarray}
    \mathrm{Maximum:} & d = & \mathrm{max}(d_1 ... d_N) \\
    \mathrm{Product:} & d = & 1 - \Prod\limits_{i=1}^{N} (1 - d_i)
\end{eqnarray}

!syntax parameters /Materials/CombinedScalarDamage

!syntax inputs /Materials/CombinedScalarDamage

!syntax children /Materials/CombinedScalarDamage

!bibtex bibliography
