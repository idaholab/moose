# ComputeLagrangianStressPK1

## Overview

This class provides an interface to define a constitutive model in 
terms of the 1st Piola-Kirchhoff stress $P_{iJ}$ and the 
associated algorithmic tangent
\begin{equation}
      T^{\prime}_{iJkL} = \frac{d P_{iJ}}{d F_{kL}}
\end{equation}
See [here](NewMaterialSystem.md) for a complete description of the Lagrangian
kernel material system and for detailed instructions on how to implement
a new material model.

## Conversion

This class converts the 1st Piola Kirchhoff stress and the
algorithmic tangent to provide the Cauchy stress,
where needed by the Lagrangian kernel system.
The conversion formula are
\begin{equation}
      P_{iJ}=J\sigma_{is}F_{Js}^{-1}
\end{equation}
and
\begin{equation}
      T_{iJkL}^{\prime}=J\sigma_{im}\left(F_{Lk}^{-1}F_{Jm}^{-1}-F_{Jk}^{-1}F_{Lm}^{-1}\right)+JT_{isab}f_{ak}^{-1}F_{Lb}^{-1}F_{Js}^{-1}
\end{equation}
