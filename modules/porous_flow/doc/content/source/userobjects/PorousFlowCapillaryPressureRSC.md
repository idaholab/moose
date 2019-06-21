# PorousFlowCapillaryPressureRSC

!syntax description /UserObjects/PorousFlowCapillaryPressureRSC

The Rogers-Stallybrass-Clements capillary relationship is [!citep](rsc1983)
\begin{equation}
S_{\mathrm{eff}} = \frac{1}{\sqrt{1 + \exp((P_{c} - A)/B)}} \ ,
\end{equation}
when the oil viscosity is exactly twice the water viscosity.  This is
of limited use in real simulations, and is only used in the Porous
Flow module for comparison with the analytical solutions offered by
the authors for multi-phase infiltration and drainage problems.

!alert note
Only effective saturation as a function of capillary pressure is available.

!syntax parameters /UserObjects/PorousFlowCapillaryPressureRSC

!syntax inputs /UserObjects/PorousFlowCapillaryPressureRSC

!syntax children /UserObjects/PorousFlowCapillaryPressureRSC


!bibtex bibliography
