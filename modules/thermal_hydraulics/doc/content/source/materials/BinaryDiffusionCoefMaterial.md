# BinaryDiffusionCoefMaterial

This material computes the mass diffusion coefficient $D_{i,j}$ for a binary mixture of gases $i$ and $j$, using the Stefan-Maxwell approximation [!citep](cunningham1980):

!equation
D_{i,j} = D_{j,i} = \frac{1}{\pi \sigma_{i,j}^2 n_m}\sqrt{\frac{2 R T_m}{\pi}\left(\frac{1}{M_i} + \frac{1}{M_j}\right)} \,,

where

- $\sigma_{i,j} = \frac{1}{2}(\sigma_i + \sigma_j)$ is the mixture collision diameter, with $\sigma_i$ denoting the collision diameter of component $i$,
- $n_m$ is the mixture concentration \[molecules/m$^3$\], which for a mixture of ideal gases, is

  !equation
  n_m = \frac{p_m}{k_B T_m} \,,

  where $p_m$ and $T_m$ are the mixture pressure and temperature, respectively, and $k_B$ is the Boltzmann constant.

- $R$ is the universal gas constant, and
- $M_i$ is the molar mass \[kg/mol\] of component $i$.

!syntax parameters /Materials/BinaryDiffusionCoefMaterial

!syntax inputs /Materials/BinaryDiffusionCoefMaterial

!syntax children /Materials/BinaryDiffusionCoefMaterial
