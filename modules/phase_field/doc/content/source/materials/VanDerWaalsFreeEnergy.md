# VanDerWaalsFreeEnergy

!syntax description /Materials/VanDerWaalsFreeEnergy

This material defines the Helmholtz free energy density $f$ of a Van der Waals gas.

\begin{equation}
f = \frac FV = \frac 1V(U - TS) = -nk_BT\left(1+\ln n_Q\left(\frac1n-b\right)\right)-n^2a
\end{equation}

where $a$ (`a`) and $b$ (`b`) are the Van der Waals coefficients, and
!include GasFreeEnergyBase.md

!syntax parameters /Materials/VanDerWaalsFreeEnergy

!syntax inputs /Materials/VanDerWaalsFreeEnergy

!syntax children /Materials/VanDerWaalsFreeEnergy
