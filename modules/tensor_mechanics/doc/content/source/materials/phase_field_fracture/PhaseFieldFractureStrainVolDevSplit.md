# Phase field fracture with strain-based volumetric-deviatoric split

## Description

This material implements the constitutive law deriving from the strain energy density
\begin{equation}
  \begin{aligned}
    \psi &= \omega(c) \psi^+ + \psi^-, \\
    \psi^+ &= \frac{1}{2}K\left< \text{Tr}(\boldsymbol{\varepsilon}^e) \right>_+^2 + \mu \text{Dev}(\boldsymbol{\varepsilon}^e) : \text{Dev}(\boldsymbol{\varepsilon}^e), \\
    \psi^- &= \frac{1}{2}K\left< \text{Tr}(\boldsymbol{\varepsilon}^e) \right>_-^2,
  \end{aligned}
\end{equation}
where $\omega(c)$ is the degradation function, $K$ is the bulk modulus, $\mu$ is the shear modulus, and $\boldsymbol{\varepsilon}^e$ is the elastic strain. $\left< \cdot \right>_\pm$ is the standard Macaulay bracket.

The stress is therefore defined as
\begin{equation}
  \begin{aligned}
    \boldsymbol{\sigma} &= \omega(c) \boldsymbol{\sigma}^+ + \boldsymbol{\sigma}^-, \\
    \boldsymbol{\sigma}^+ &= K\left< \text{Tr}(\boldsymbol{\varepsilon}^e) \right>_+\boldsymbol{I} + 2\mu\text{Dev}(\boldsymbol{\varepsilon}^e), \\
    \boldsymbol{\sigma}^+ &= K\left< \text{Tr}(\boldsymbol{\varepsilon}^e) \right>_-\boldsymbol{I}.
  \end{aligned}
\end{equation}

!syntax parameters /Materials/PhaseFieldFractureStrainSpectralSplit
