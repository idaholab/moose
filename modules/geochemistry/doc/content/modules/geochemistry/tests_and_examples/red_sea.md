# Chemical model of Red Sea brine

This example closely follows Section 6.3 of [!cite](bethke_2007).

A chemical analysis of the major element composition of hot hydrothermal brines of the Red Sea is shown in [table:analysis].  In addition, the temperature is around 60$^{\circ}$C and the pH is 5.6.

!table id=table:analysis caption=Major element composition of Red Sea brine
| Species | Concentration (mg.kg$^{-1}$) |
| --- | --- |
| Cl$^{-}$ | 156030 |
| Na$^{+}$ | 92600 |
| Ca$^{2+}$ | 5150 |
| K$^{+}$ | 1870 |
| SO$_{4}^{2-}$ | 840 |
| Mg$^{2+}$ | 764 |
| HCO$_{3}^{-}$ | 140 |
| Fe$^{2+}$ | 81 |
| Zn$^{2+}$ | 5.4 |
| F$^{-}$ | 5 |
| Ba$^{2+}$ | 0.9 |
| Pb$^{2+}$ | 0.63 |
| Cu$^{+}$ | 0.26 |

In order to estimate the brine's oxidation state, [!cite](bethke_2007) recommends using the mineral Sphalerite instead of primary species O2(aq), and the mineral Barite instead of primary aqueous species Ba++.  [!cite](bethke_2007) uses a small initial value of $10^{-9}\,$g of free amounts of each of these minerals to avoid dissolution when considering precipitation/dissolution later int he calculation.


## Species distribution

Assume the extent of the system is 1$\,$kg of solvent water along with the solutes dissolved in it.  The mass of the system is 1.25$\,$kg.  MOOSE produces the result ????

Ignoring all mineral supersaturation, [!cite](bethke_2007) states that an equilibrium calculation of the molalities of the most abundant species results in [table:molalities_etc].  These may be compared with the prediction from MOOSE in ????

!table id=table:molalities_etc caption=Calculated molalities, activity coefficients and activities of the most abundant species in Red Sea water
| Species | Molality (mol.kg$^{-1}$) | Activity coeff | log$_{10}$a |
| --- | --- | --- | --- |
| Cl$^{-}$ | 5.183 | 0.6125 | 0.5017 | 
| Na$^{+}$ | 4.861 | 0.7036 | 0.5341 |
| NaCl | 0.5512 | 1.0 | -0.2587 |
| CaCl$^{+}$ | 0.1276 | 0.7036 | -1.047|
| K$^{+}$ | $0.5951\times 10^{-1}$ | 0.6125 | -1.438 |
| Ca$^{2+}$ | $0.4445\times 10^{-1}$ | 0.1941 | -2.064 |
| MgCl$^{+}$ | $0.2496\times 10^{-1}$ | 0.7036 | -1.756 |
| Mg$^{2+}$ | $0.1692\times 10^{-1}$ | 0.2895 | -2.310 |
| NaSO$_{4}^{-}$ | $0.7906\times 10^{-2}$ | 0.7036 | -2.325 |
| SO$_{4}^{2-}$ | $0.2675\times 10^{-2}$ | 0.0985 | -3.579 |
| CO$_{2}$(aq) | $0.1809\times 10^{-2}$ | 1.0 | -2.743 |


## Mass conservation

Note that the free species concentrations do not satisfy the input constraints, since the latter are bulk values.  Nevertheless, mass conservation may be checked as in Section 6.1.3 of [!cite](bethke_2007).  Performing this calculation using MOOSE produces ????

## Mass action

Similarly, the mass-action equations may be checked as in Section 6.1.3 of [!cite](bethke_2007).  Performing this calculation using MOOSE produces ????

## Charge balance

MOOSE produces a total free charge of ????

## Minerals

The saturation indices of the equilibrium solution in [table:molalities_etc] are greater than 0 for a number of minerals: bornite, chalcopyrite, chalcocite, pyrite, flourite, galena and covellite.  MOOSE produces the result ????

## Mineral precipitation

Allowing the minerals to precipitate [!cite](bethke_2007) predicts that 3 minerals form in small quantities, as shown in [table:minerals]

!table id=table:minerals caption=Calculated final mass of each precipitate in the stable phase assemblage for the Red Sea brine.
| Mineral | Mass (g) |
| --- | --- | --- |
| Fluorite | $7.3\times 10^{-3}$ |
| Chalcocite | $9.3\times 10^{-6}$ |
| Barite | $5.8\times 10^{-8}$ |

!listing modules/geochemistry/test/tests/equilibrium_models/red_sea_no_precip.i

!listing modules/geochemistry/test/tests/equilibrium_models/red_sea_precip.i

!bibtex bibliography