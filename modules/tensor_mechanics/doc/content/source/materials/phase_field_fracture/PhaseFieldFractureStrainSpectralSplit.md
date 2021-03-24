# Phase field fracture with strain-based spectral split

## Description

This material implements the constitutive law deriving from the strain energy density
\begin{equation}
  \begin{aligned}
    \psi &= \omega(c) \psi^+ + \psi^-, \\
    \psi^+ &= \frac{1}{2}\lambda\left< \text{Tr}(\boldsymbol{\varepsilon}^e) \right>_+^2 + \mu \boldsymbol{\varepsilon}^{e^+} : \boldsymbol{\varepsilon}^{e^+}, \\
    \psi^- &= \frac{1}{2}\lambda\left< \text{Tr}(\boldsymbol{\varepsilon}^e) \right>_-^2 + \mu \boldsymbol{\varepsilon}^{e^-} : \boldsymbol{\varepsilon}^{e^-},
  \end{aligned}
\end{equation}
where $\omega(c)$ is the degradation function, $\lambda$ is the Lame constant, $\mu$ is the shear modulus, and $\boldsymbol{\varepsilon}^e$ is the elastic strain. $\left< \cdot \right>_\pm$ is the standard Macaulay bracket.

$\boldsymbol{\varepsilon}^{e^\pm}$ are the elastic strain in the positive/negative spectrum, defined as
\begin{equation}
  \begin{aligned}
    \boldsymbol{\varepsilon}^{e} &= \sum_{i=1}^3 \varepsilon_i \boldsymbol{n}_i \otimes \boldsymbol{n}_i, \\
    \boldsymbol{\varepsilon}^{e^+} &= \sum_{i=1}^3 \left<\varepsilon_i\right>_+ \boldsymbol{n}_i \otimes \boldsymbol{n}_i, \\
    \boldsymbol{\varepsilon}^{e^-} &= \sum_{i=1}^3 \left<\varepsilon_i\right>_- \boldsymbol{n}_i \otimes \boldsymbol{n}_i,
  \end{aligned}
\end{equation}
where $\varepsilon_i$ and $\boldsymbol{n}_i$ are eigenvalues and eigenvectors of the elastic strain.

The stress is therefore defined as
\begin{equation}
  \begin{aligned}
    \boldsymbol{\sigma} &= \omega(c) \boldsymbol{\sigma}^+ + \boldsymbol{\sigma}^-, \\
    \boldsymbol{\sigma}^+ &= \lambda\left< \text{Tr}(\boldsymbol{\varepsilon}^e) \right>_+\boldsymbol{I} + 2\mu\boldsymbol{\varepsilon}^{e^+}, \\
    \boldsymbol{\sigma}^+ &= \lambda\left< \text{Tr}(\boldsymbol{\varepsilon}^e) \right>_-\boldsymbol{I} + 2\mu\boldsymbol{\varepsilon}^{e^-}.
  \end{aligned}
\end{equation}

!syntax parameters /Materials/PhaseFieldFractureStrainSpectralSplit
