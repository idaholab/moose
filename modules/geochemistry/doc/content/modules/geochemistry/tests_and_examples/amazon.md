# Chemical model of Amazon river water

This example closely follows Section 6.2 of [!cite](bethke_2007).

A chemical analysis of the major element composition of water in the Amazon river [table:analysis].  The "free" O$_{2}$(aq) concentration of 5.8$\,$mg.kg$^{-1}$ means its bulk composition will be more than this. In addition, the pH is 6.5.

!table id=table:analysis caption=Major element composition of Amazon river water
| Species | Concentration (mg.kg$^{-1}$) |
| --- | --- |
| HCO$_{3}^{-}$ | 19 |
| SiO$_{2}$(aq) | 7 |
| O$_{2}$(aq) | 5.8 free |
| Ca$^{2+}$ | 4.3 |
| SO$_{4}^{2-}$ | 3 |
| Cl$^{-}$ | 1.9 |
| Na$^{+}$ | 1.8 | 
| Mg$^{2+}$ | 1.1 |
| Al$^{3+}$ | 0.07 |
| Fe$^{2+}$ | 0.06 |

## Species distribution

Assume the extent of the system is 1$\,$kg of solvent water along with the solutes dissolved in it.  Charge balance is performed on Cl$^{-}$.

Ignoring all mineral supersaturation, [!cite](bethke_2007) states that an equilibrium calculation of the molalities of the most abundant species results in [table:molalities_etc].  These may be compared with the prediction from MOOSE in ????

!table id=table:molalities_etc caption=Calculated molalities, activity coefficients and activities of the most abundant species in Amazon river water
| Species | Molality (mol.kg$^{-1}$) | Activity coeff | log$_{10}$a |
| --- | --- | --- | --- |
| HCO$_{3}^{-}$ | $0.181\times 10^{-3}$ | 0.974 | -3.75 |
| O$_{2}$(aq) | $0.181\times 10^{-3}$ | 1.000 | -3.74 | 
| Cl$^{-}$ | $0.138\times 10^{-3}$ | 0.973 | -3.87 |
| CO$_{2}$(aq) | $0.130\times 10^{-3}$ | 1.000 | -3.89 |
| SiO$_{2}$(aq) | $0.116\times 10^{-3}$ | 1.000 | -3.93 |
| Ca$^{2+}$ | $0.106\times 10^{-3}$ | 0.899 | -4.02 |
| Na$^{+}$ | $0.783\times 10^{-4}$ | 0.973 | -4.12 |
| Mg$^{2+}$ | $0.450\times 10^{-4}$ | 0.901 | -4.39 |
| SO$_{4}^{2-}$ | $0.305\times 10^{-4}$ | 0.898 | -4.56 |

## Mass conservation

Note that the free species concentrations do not satisfy the input constraints, since the latter are bulk values.  Nevertheless, mass conservation may be checked as in Section 6.1.3 of [!cite](bethke_2007).  Performing this calculation using MOOSE produces ????

## Mass action

Similarly, the mass-action equations may be checked as in Section 6.1.3 of [!cite](bethke_2007).  Performing this calculation using MOOSE produces ????

## Charge balance

MOOSE produces a total free charge of ????

## Minerals

The saturation indices of the equilibrium solution in [table:molalities_etc] are greater than 0 for the minerals: nontronite, hematitie, kaolinite, beidellite, pyrophyllite, gibbsite, paragonite and quartz (see Figure 6.3 of [!cite](bethke_2007)).  MOOSE produces the result ????

## Fixing mineral volumes

An alternate model assumes the water is in equilibrium with kaolinite and hematite, and that these control the aluminum and iron concentrations.  That is, instead of Al$^{3+}$, kaolinite is used in the basis, and instead of Fe$^{2+}$, hematite is used in the basis.  The free volumes of each of these is rather unimportant, but [!cite](bethke_2007) suggests using the values in [table:mineral_fixed].

!table id=table:mineral_fixed caption=Alternate model of Amazon river water assumes equilibrium with kaoline and hematite with specified amount
| Mineral | Free amount (cm$^{3}$) |
| --- | --- |
| Kaolinite | 1 |
| Hematite | 1 |

According to this model, only nontronite clay is supersaturated by any significant amount.  MOOSE produces the result ????

!listing modules/geochemistry/test/tests/equilibrium_models/amazon.i

!listing modules/geochemistry/test/tests/equilibrium_models/amazon_with_minerals.i


!bibtex bibliography