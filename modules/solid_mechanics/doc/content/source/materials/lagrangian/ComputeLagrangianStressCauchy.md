# ComputeLagrangianStressCauchy

## Overview

This class provides an interface to define a constitutive model in 
terms of the Cauchy stress stress $\sigma_{ij}$ and the 
associated algorithmic tangent
\begin{equation}
      T_{ijkl} = \frac{d \sigma_{ij}}{d \Delta l_{kl}}
\end{equation}
See [here](NewMaterialSystem.md) for a complete description of the Lagrangian
kernel material system and for detailed instructions on how to implement
a new material model.

## Conversion

This class converts the Cauchy stress and the algorithmic 
tangent to provide the 1st Piola Kirchhoff stress, where needed by the
Lagrangian kernel system.  The conversion formula are:
\begin{equation}
      \sigma_{ij}=\frac{1}{J}P_{iK}F_{jK}
\end{equation}
and
\begin{equation}
      T_{ijkl}=\frac{d\sigma_{ij}}{d\Delta l_{kl}}=\frac{1}{J}T_{iAmN}^{\prime}F_{jA}F_{lN}f_{mk}+f_{jk}\sigma_{il}-\sigma_{ij}f_{lk}
\end{equation}
