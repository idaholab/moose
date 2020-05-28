# Chemical models of Morro de Ferro groundwater

This example closely follows Section 7.3 of [!cite](bethke_2007).

A chemical analysis of the major element composition of Morro de Ferro groundwater is shown in [table:analysis].  In addition:

- the temperature is 22$^{\circ}$C;
- the pH is 6.05;
- Eh$=504\,$mV.

!table id=table:analysis caption=Major element composition of Morro de Ferror groundwater
| Species | Concentration (mg.litre$^{-1}$) |
| --- | --- |
| HCO$_{3}^{-}$ | 1.8 |
| Ca$^{2+}$ | 0.238 |
| Mg$^{2+}$ | 0.352 |
| Na$^{+}$ | 0.043 |
| K$^{+}$ | 0.20 |
| Fe (II) | 0.73 |
| Fe (total) | 0.76 |
| Mn$^{2+}$ | 0.277 |
| Zn$^{2+}$ | 0.124 |
| Cl$^{-}$ | $<2.0$ |
| SO$_{4}^{2-}$ | 0.15 |
| Dissolved O$_{2}$ | 4.3 |

## Setting oxidation using dissolved oxygen, and assuming redox equilibrium

If:

- redox equilibrium for iron is assumed,
- the free concentration of O$_{2}$(aq) is set to 4.3$\,$mg.litre$^{-1}$,
- the bulk composition of Fe$^{2+}$ is 0.73$\,$mg.litre$^{-1}$,
- and charge balance is enforced on Cl$^{-}$

then the species distribution predicted by [!cite](bethke_2007) is shown in the second column of [table:molalities_etc].  This result is obtained by ignoring any potential precipitation of minerals.  MOOSE produces the result ????

## Assuming redox disequilibrium for iron

If:

- redox disequilibrium for iron is assumed,
- the free concentration of O$_{2}$(aq) is set to 4.3$\,$mg.litre$^{-1}$,
- the bulk composition of Fe$^{2+}$ is 0.73$\,$mg.litre$^{-1}$,
- the bulk composition of Fe$^{3+}$ is 0.03$\,$mg.litre$^{-1}$,
- and charge balance is enforced on Cl$^{-}$

then the species distribution predicted by [!cite](bethke_2007) is shown in the right-hand column of [table:molalities_etc].  This result is obtained by ignoring any potential precipitation of minerals.  MOOSE produces the result ????

!table id=table:molalities_etc caption=Calculated molalities (mol.kg$^{-1}$) of iron species in Morro de Ferro groundwater, assuming redox equilibrium (central column) or disequilibrium (right-hand column)
| Species | Equilibrium | Disequilibrium |
| --- | --- | --- | --- |
| Fe$^{2+}$ | $0.11\times 10^{-12}$ | $0.13\times 10^{-4}$ |
| FeSO$_{4}$ | $0.24\times 10^{-15}$ | $0.32\times 10^{-7}$ |
| FeHCO$_{3}^{-}$ | $0.20\times 10^{-16}$ | $0.24\times 10^{-8}$ |
| FeCl$^{+}$ | $0.66\times 10^{-17}$ | $0.12\times 10^{-8}$ |
| FeOH$^{+}$ | $0.64\times 10^{-17}$ | $0.77\times 10^{-9}$ |
| Fe(OH)$_{2}^{+}$ | $0.90\times 10^{-5}$ | $0.37\times 10^{-6}$ |
| Fe(OH)$_{3}$ | $0.38\times 10^{-5}$ | $0.16\times 10^{-6}$ |
| FeOH$^{2+}$ | $0.29\times 10^{-7}$ | $0.12\times 10^{-8}$ |
| Fe(OH)$_{4}^{-}$ | $0.91\times 10^{-9}$ | $0.37\times 10^{-10}$ |



## Mass conservation

Note that the free species concentrations do not satisfy the input constraints, since the latter are bulk values.  Nevertheless, mass conservation may be checked as in Section 6.1.3 of [!cite](bethke_2007).  Performing this calculation using MOOSE produces ????

## Mass action

Similarly, the mass-action equations may be checked as in Section 6.1.3 of [!cite](bethke_2007).  Performing this calculation using MOOSE produces ????

## Charge balance

MOOSE produces a total free charge of ????

## Nernst Eh values

[!cite](bethke_2007) computes the Nernst Eh values for the redox half-reactions as:
\begin{equation}
\begin{aligned}
\frac{1}{4}\mathrm{O}_{2}\mathrm{(aq)} + \mathrm{H}^{+} + \mathrm{e}^{-} \rightleftharpoons \frac{1}{2}\mathrm{H}_{2}\mathrm{O} && \mathrm{Eh}=861\,\mathrm{mV} \\
\mathrm{Fe}^{3+} + \mathrm{e}^{-} \rightleftharpoons \mathrm{Fe}^{2+} && \mathrm{Eh}=306\,\mathrm{mV}
\end{aligned}
\end{equation}
MOOSE produces the result of ????

!listing modules/geochemistry/test/tests/redox_disequilibrium/morro.i

!listing modules/geochemistry/test/tests/redox_disequilibrium/morro_disequilibrium.i

!bibtex bibliography