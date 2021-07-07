# Phase field fracture with stress-based spectral split

## Description

This material implements the constitutive law deriving from the strain energy density
\begin{equation}
  \begin{aligned}
    \psi &= \omega(c) \psi^+ + \psi^-, \\
    \psi^+ &= \frac{1}{2}\boldsymbol{\sigma}^+ : \boldsymbol{\varepsilon}^e, \\
    \psi^- &= \frac{1}{2}\boldsymbol{\sigma}^- : \boldsymbol{\varepsilon}^e,
  \end{aligned}
\end{equation}
where $\omega(c)$ is the degradation function, $\varepsilon{\sigma}$ is the stress, and $\boldsymbol{\varepsilon}^e$ is the elastic strain. $\left< \cdot \right>_\pm$ is the standard Macaulay bracket.

$\boldsymbol{\sigma}^\pm$ are the stress in the positive/negative spectrum, defined as
\begin{equation}
  \begin{aligned}
    \boldsymbol{\sigma} &= \sum_{i=1}^3 \sigma_i \boldsymbol{n}_i \otimes \boldsymbol{n}_i, \\
    \boldsymbol{\sigma}^+ &= \sum_{i=1}^3 \left<\sigma_i\right>_+ \boldsymbol{n}_i \otimes \boldsymbol{n}_i, \\
    \boldsymbol{\sigma}^- &= \sum_{i=1}^3 \left<\sigma_i\right>_- \boldsymbol{n}_i \otimes \boldsymbol{n}_i,
  \end{aligned}
\end{equation}
where $\sigma_i$ and $\boldsymbol{n}_i$ are eigenvalues and eigenvectors of the stress.

The stress is defined as
\begin{equation}
  \boldsymbol{\sigma} = \lambda\text{Tr}(\boldsymbol{\varepsilon}^e)\boldsymbol{I} + 2\mu\boldsymbol{\varepsilon}^e.
\end{equation}

!syntax parameters /Materials/PhaseFieldFractureStressSpectralSplit
