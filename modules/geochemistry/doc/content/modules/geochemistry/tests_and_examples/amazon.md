# Chemical model of Amazon river water

This example closely follows Section 6.2 of [!cite](bethke_2007).

A chemical analysis of the major element composition of water in the Amazon river [table:analysis].  The "free" O$_{2}$(aq) concentration of 5.8$\,$mg.kg$^{-1}$ means its bulk composition will be more than this. In addition, the pH is 6.5.  Assume that the extent of the system is 1$\,$kg of solvent water along with the solutes dissolved in it.

!table id=table:analysis caption=Major element composition of Amazon river water 
| Species | Concentration (mg.kg$^{-1}$) |
| --- | --- |
| HCO$_{3}^{-}$ | 19 |
| SiO$_{2}$(aq) | 7 |
| O$_{2}$(aq) | 5.8 free |
| Ca$^{2+}$ | 4.3 |
| SO$_{4}^{2-}$ | 3 |
| Cl$^{-}$ | 4.9 |
| Na$^{+}$ | 1.8 | 
| Mg$^{2+}$ | 1.1 |
| Al$^{3+}$ | 0.07 |
| Fe$^{2+}$ | 0.06 |

The concentration of Cl$^{-}$ is listed as 1.9$\,$mg.kg$^{-1}$ in [!cite](bethke_2007), but its molality is computed to be 1.38E-4$\,$mol/kg(solvent water).  Hence its bulk composition must have been at least 1.38E-4$\,$mol since all the stoichiometric coefficients of Cl- in all the secondary species are positive.  This implies the bulk composition of Cl$^{-}$ must be more than 1.9$\,$mg/kg(soln).  There is probably a typo in the book.  Hence the bulk concentration is set to 4.9$\,$mg/kg(soln), leading to 1.383E-4$\,$moles.   In any case, charge balance is performed on Cl$^{-}$.

## MOOSE input file: no minerals

The MOOSE input file contains the usual [GeochemicalModelDefinition](GeochemicalModelDefinition.md) that specifies the database file to use, and in this case just the basis species and equilibrium minerals (that are prevented from precipitating, but for which the saturation indices are of interest).  The flag `piecewise_linear_interpolation = true` in order to compare with the [Geochemists Workbench](https://www.gwb.com/) result.

!listing modules/geochemistry/test/tests/equilibrium_models/amazon.i block=UserObjects

To instruct MOOSE to find the equilibrium configuration, a [TimeIndependentReactionSolver](actions/AddTimeDependentReactionSolverAction.md) is used:

- The pH is fixed by setting the activity of H$^{+}$.
- The bulk mole number of the aqueous species is also fixed appropriately.  The numbers are different than the concentration in mg.kg$^{-1}$ given in the above table, and may be worked out using the [TDS](tests_and_examples/ic_unit_conversions.md).
- The `prevent_precipitation` input prevents any minerals from precipitating when finding the equilibrium configuration, even if their saturation indices are positive.
- The other flags enable an accurate comparison with the [Geochemists Workbench](https://www.gwb.com/) software.

!listing modules/geochemistry/test/tests/equilibrium_models/amazon.i block=TimeIndependentReactionSolver

## GWB input file: no minerals

The analogous [GWB](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/equilibrium_models/amazon.rea

## Results: no minerals

### Error and charge-neutrality error

The `geochemistry` simulation reports an error of 3.163e-16$\,$mol, and that the charge of the solution is 0$\,$mol.

### Solution mass

The solution mass is 1.000$\,$kg.

### Ionic strength and water activity

The ionic strength is 0.0005644$\,$mol/kg(solvent water), and the water activity is 1.000.

### pH, pe and Eh

The pH is 6.5, the pe is 14.07, and Eh = 0.832$\,$V.

### Species distribution

Ignoring all mineral supersaturation, [!cite](bethke_2007) states that an equilibrium calculation of the molalities of the most abundant species results in [table:molalities_etc].

Identical results are produced by the `geochemistry` module and the [Geochemists Workbench](https://www.gwb.com/) software.

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

### Minerals

Both the `geochemistry` module and the GWB software predict indentical saturation indices for minerals.  They predict the saturation indices of the equilibrium solution in [table:molalities_etc] are greater than 0 for the minerals: nontronite, hematitie, kaolinite, beidellite, pyrophyllite, gibbsite, paragonite and quartz (see Figure 6.3 of [!cite](bethke_2007)).

## Fixing mineral volumes

An alternate model assumes the water is in equilibrium with kaolinite and hematite, and that these control the aluminum and iron concentrations.  That is, instead of Al$^{3+}$, kaolinite is used in the basis, and instead of Fe$^{2+}$, hematite is used in the basis.  The free volumes of each of these is rather unimportant, but [!cite](bethke_2007) suggests using the values in [table:mineral_fixed].

!table id=table:mineral_fixed caption=Alternate model of Amazon river water assumes equilibrium with kaoline and hematite with specified amount
| Mineral | Free amount (cm$^{3}$) |
| --- | --- |
| Kaolinite | 1 |
| Hematite | 1 |

## MOOSE input file: with minerals

!listing modules/geochemistry/test/tests/equilibrium_models/amazon_with_minerals.i

## GWB input file: with minerals

!listing modules/geochemistry/test/tests/equilibrium_models/amazon_with_minerals.rea

## Results: with minerals

According to this model, only nontronite clay is supersaturated by any significant amount.  The `geochemistry` module produces identical results to the [GWB](https://www.gwb.com/) software.



!bibtex bibliography