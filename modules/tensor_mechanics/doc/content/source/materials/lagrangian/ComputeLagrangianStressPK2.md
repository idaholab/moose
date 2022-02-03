# ComputeLagrangianStressPK2

## Overview

This class provides an interface to define a constitutive model in 
terms of the 2nd Piola-Kirchhoff stress $S_{IJ}$ and the 
associated algorithmic tangent
\begin{equation}
      T^{\prime\prime}_{IJKL} = \frac{d S_{IJ}}{d S_{KL}}
\end{equation}
It provides the Green-Lagrange strain
\begin{equation}
      E_{IJ} = \frac{1}{2}\left(F_{kI}F_{kJ} - \delta_{IJ}\right)
\end{equation}
as an additional kinematic measure available for the user to use in
the course of the defining the stress update.
See [here](NewMaterialSystem.md) for a complete description of the Lagrangian
kernel material system and for detailed instructions on how to implement
a new material model.

## Conversion

The class inherits from [`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md)
and so must map from the 2nd Piola Kirchhoff stress to the 1st Piola
Kirchhoff stress (and similarly map the tangents).
The conversion formula are
\begin{equation}
      P_{iJ} = F_{iK} S_{KJ}
\end{equation}
and
\begin{equation}
      T_{iJkL}^{\prime}=\delta_{ik}S_{LJ}+F_{iT}T_{TJMN}^{\prime\prime}\frac{1}{2}\left(\delta_{ML}F_{kN}+F_{kM}\delta_{NL}\right)
\end{equation}
