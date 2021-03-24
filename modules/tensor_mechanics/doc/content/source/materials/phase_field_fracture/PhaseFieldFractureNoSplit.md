# Phase field fracture without energy split

## Description

This material implements the constitutive law deriving from the strain energy density
\begin{equation}
  \psi = \omega(c) \frac{1}{2} \boldsymbol{\varepsilon}^e : \mathbb{C} : \boldsymbol{\varepsilon}^e,
\end{equation}
where $\omega(c)$ is the degradation function, $\mathbb{C}$ is the elasticity tensor, and $\boldsymbol{\varepsilon}^e$ is the elastic strain. The stress is therefore defined as
\begin{equation}
  \boldsymbol{\sigma} = \omega(c) \mathbb{C} : \boldsymbol{\varepsilon}^e.
\end{equation}

!syntax parameters /Materials/PhaseFieldFractureNoSplit
