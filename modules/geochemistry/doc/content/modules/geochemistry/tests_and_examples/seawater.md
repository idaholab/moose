# Chemical model of seawater

This example closely follows Section 6.1 of [!cite](bethke_2007).

A chemical analysis of the major element composition of seawater is shown in [table:analysis], while [table:atm] lists the partial pressures of some gases in the atmosphere.

!table id=table:analysis caption=Major element composition of seawater
| Species | Concentration (mg.kg$^{-1}$) |
| --- | --- |
| Cl$^{-}$ | 19350 |
| Na$^{+}$ | 10760 |
| SO$_{4}^{2-}$ | 2710 |
| Mg$^{2+}$ | 1290 |
| Ca$^{2+}$ | 411 |
| K$^{+}$ | 399 |
| HCO$_{3}^{-}$ | 142 |
| SiO$_{2}$(aq) | 0.1--10 |
| O$_{2}$(aq) | 0.1--6 |

!table id=table:atm caption=Partial pressures of some gases in the atmosphere
| Gas | Pressure (atm) |
| --- | --- |
| N$_{2}$ | 0.78 |
| O$_{2}$ | 0.21 |
| H$_{2}$O | 0.001--0.23 |
| CO$_{2}$ | 0.0003 |
| CH$_{4}$ | $1.5\times 10^{-6}$ |
| CO | $(0.06-1)\times 10^{-6}$ |
| SO$_{2}$ | $1\times 10^{-6}$ |
| N$_{2}$O | $5\times 10^{-7}$ |
| H$_{2}$ | $\sim 5\times 10^{-7}$ |
| NO$_{2}$ | $(0.05-2)\times 10^{-8}$ |

In this example, we assume that the CO$_{2}$(g) and O$_{2}$(g) fugacities can be set to approximately their partial pressures.  Fixing the CO$_{2}$ fugacity fixes pH according to the reaction
\begin{equation}
\mathrm{H}^{+} + \mathrm{HCO}_{3}^{-} \rightleftharpoons \mathrm{CO}_{2}\mathrm{(g)} + \mathrm{H}_{2}\mathrm{O}
\end{equation}
Fixing the fugacity of O$_{2}$(g) defines the oxidation state according to
\begin{equation}
\mathrm{O}_{2}\mathrm{(aq)} \rightleftharpoons \mathrm{O}_{2}\mathrm{(g)}
\end{equation}

## Species distribution

Assume the extent of the system is 1$\,$kg of solvent water along with the solutes dissolved in it.  This means the solution mass is 1.0364$\,$kg.  The value outputted by MOOSE is ????

Ignoring all mineral supersaturation, [!cite](bethke_2007) states that an equilibrium calculation of the molalities of the most abundant species results in [table:molalities_etc].  These may be compared with the prediction from MOOSE in ????

!table id=table:molalities_etc caption=Calculated molalities, activity coefficients and activities of the most abundant species in seawater
| Species | Molality (mol.kg$^{-1}$) | Activity coeff | log$_{10}$a |
| --- | --- | --- | --- |
| Cl$^{-}$ | 0.5500 | 0.6276 | -0.4619 |
| Na$^{+}$ | 0.4754 | 0.6717 | -0.4958 |
| Mg$^{2+}$ | $0.3975\times 10^{-1}$ | 0.3160 | -1.9009 |
| SO$_{4}^{2-}$ | $0.1607\times 10^{-1}$ | 0.1692 | -2.5657 |
| K$^{+}$ | $0.1033\times 10^{-1}$ | 0.6276 | -2.1881 |
| MgCl$^{+}$ | $0.9126\times 10^{-2}$ | 0.6717 | -2.2125 |
| NaSO$_{4}^{-}$ | $0.6386\times 10^{-2}$ | 0.6717 | -2.3676 |
| Ca$^{2+}$ | $0.5953\times 10^{-2}$ | 0.2465 | -2.8334 |
| MgSO$_{4}$ | $0.5767\times 10^{-2}$ | 1.0 | -2.2391 |
| CaCl$^{+}$ | $0.3780\times 10^{-2}$ | 0.6717 | -2.5953 |
| NaCl | $0.2773\times 10^{-2}$ | 1.0 | -2.5571 |
| HCO$_{3}^{-}$ | $0.1498\times 10^{-2}$ | 0.6906 | -2.9851 |
| CaSO$_{4}$ | $0.8334\times 10^{-3}$ | 1.0 | -3.0792 |
| NaHCO$_{3}$ | $0.4447\times 10^{-3}$ | 1.0 | -3.3519 |
| O$_{2}$(aq) | $0.2151\times 10^{-3}$ | 1.1735 | -3.5980 |
| MgHCO$_{3}^{+}$ | $0.1981\times 10^{-3}$ | 0.6717 | -3.8760 |
| KSO$_{4}^{-}$ | $0.1869\times 10^{-3}$ | 0.6717 | -3.9013 |
| MgCO$_{3}$ | $0.1068\times 10^{-3}$ | 1.0 | -3.9715 |
| SiO$_{2}$(aq) | $0.8188\times 10^{-4}$ | 1.1735 | -4.0174 |
| KCl | $0.5785\times 10^{-4}$ | 1.0 | -4.2377 |
| CO$_{3}^{2-}$ | $0.5437\times 10^{-4}$ | 0.1891 | -4.9879 |

## pH

[!cite](bethke_2007) predicts the pH to be 8.34.  MOOSE produces the result ????

## Mass conservation

Note that the free species concentrations do not satisfy the input constraints, since the latter are bulk values.  Nevertheless, mass conservation may be checked as in Section 6.1.3 of [!cite](bethke_2007).  Performing this calculation using MOOSE produces ????

## Mass action

Similarly, the mass-action equations may be checked as in Section 6.1.3 of [!cite](bethke_2007).  Performing this calculation using MOOSE produces ????

## Charge balance

MOOSE produces a total free charge of ????

## Minerals

The saturation indices of the equilibrium solution in [table:molalities_etc] are greater than 0 for a number of minerals, which suggests some minerals will precipitate.  Allowing this to occur, [!cite](bethke_2007) finds that only dolomite and quartz actually precipitate, as shown in [table:minerals].  MOOSE produces the result ????

!table id=table:minerals caption=Calculated initial saturation indices, and the final mass of each precipitate in the stable phase assemblage
| Mineral | Initial SI | Amount formed (mg) |
| --- | --- | --- |
| Antigorite | 44.16 | 0 |
| Tremolite | 7.73 | 0 |
| Talc | 6.68 | 0 |
| Chrysotile | 4.72 | 0 |
| Sepiolite | 3.93 | 0 |
| Anthophyllite | 3.48 | 0 |
| Dolomite | 3.46 | 50 |
| Dolomite-ord | 3.46 | 0 |
| Huntite | 2.13 | 0 |
| Dolomite-dis | 1.91 | 0 |
| Magnesite | 1.02 | 0 |
| Calcite | 0.81 | 0 |
| Aragonite | 0.64 | 0 |
| Quartz | -0.02 | 1 |

!listing modules/geochemistry/test/tests/equilibrium_models/seawater_no_precip.i

!listing modules/geochemistry/test/tests/equilibrium_models/seawater_precip.i

!bibtex bibliography