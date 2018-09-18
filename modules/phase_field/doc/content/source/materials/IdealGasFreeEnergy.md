# IdealGasFreeEnergy

!syntax description /Materials/IdealGasFreeEnergy

This material defines the Helmholtz free energy density $f$ of an ideal Boltzmann gas.

\begin{equation}
f = \frac FV = \frac 1V(U - TS) = -nk_BT\left(1+\ln\frac {n_Q}n\right)
\end{equation}

where
!include GasFreeEnergyBase.md

!syntax parameters /Materials/IdealGasFreeEnergy

!syntax inputs /Materials/IdealGasFreeEnergy

!syntax children /Materials/IdealGasFreeEnergy
