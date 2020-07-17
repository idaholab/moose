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
Finally, assume the extent of the system is 1$\,$kg of solvent water along with the solutes dissolved in it.

## MOOSE input file: no precipitation

The MOOSE input file contains the usual [GeochemicalModelDefinition](GeochemicalModelDefinition.md) that specifies the database file to use, and in this case the basis species, equilibrium minerals and equilibrium gases.  The flag `piecewise_linear_interpolation = true` in order to compare with the Geochemists Workbench result

!listing modules/geochemistry/test/tests/equilibrium_models/seawater_no_precip.i block=UserObjects

To instruct MOOSE to find the equilibrium configuration, a [TimeIndependentReactionSolver](actions/AddTimeIndependentReactionSolverAction.md) is used:

- The swaps are defined.
- The fugacity of the gases is fixed as defined above.
- The bulk mole number of the aqueous species is also fixed appropriately.  The numbers are different than the concentration in mg.kg$^{-1}$ given in the above table, and may be worked out using the [TDS](tests_and_examples/ic_unit_conversions.md).
- The `prevent_precipitation` input prevents any minerals from precipitating when finding the equilibrium configuration, even if their saturation indices are positive.
- The other flags enable an accurate comparison with the [Geochemists Workbench](https://www.gwb.com/) software.

!listing modules/geochemistry/test/tests/equilibrium_models/seawater_no_precip.i block=TimeIndependentReactionSolver

## MOOSE input file: with precipitation

The MOOSE input file is very similar to the no-precipitation case.  There are two differences:

- The `prevent_precipitation` input is changed.
- The swaps are different, as is the initial condition for MgCO$_{3}$.  This is discussed in [!cite](bethke_2007). 

!listing modules/geochemistry/test/tests/equilibrium_models/seawater_precip.i


## Geochemists Workbench input files

The [Geochemists Workbench](https://www.gwb.com/) input file for the precipitation case is:

!listing modules/geochemistry/test/tests/equilibrium_models/seawater_precip.rea

The non-precipitation case is similar (see the seawater_no_precip.rea file).

## Results

The `geochemistry` results mirror those from Geochemists Workbench exactly.

### Error and charge-neutrality error

The `geochemistry` simulation reports an error of 1.664e-16$\,$mol, and that the charge of the solution is 4.467e-17$\,$mol.

### Solution mass

The solution mass is 1.036$\,$kg.

### Ionic strength and water activity

The ionic strength is 0.6518$\,$mol/kg(solvent water), and the water activity is 0.982.

### pH, pe and Eh

After precipitation, the pH is 6.726, the pe is 13.88, and Eh = 0.821$\,$V.

### Aqueous species distribution

The molalities of the most abundant species results in [table:molalities_etc].

!table id=table:molalities_etc caption=Calculated molalities, activity coefficients and activities of the most abundant species in seawater
| Species | Molality (mol.kg$^{-1}$) | Activity coeff | log$_{10}$a |
| --- | --- | --- | --- |
| Cl$^{-}$ | 0.5503 | 0.6276 | -0.4617 |
| Na$^{+}$ | 0.4755 | 0.6717 | -0.4957 |
| Mg$^{2+}$ | $0.3984\times 10^{-1}$ | 0.3160 | -1.9000 |
| SO$_{4}^{2-}$ | $0.1605\times 10^{-1}$ | 0.1692 | -2.5661 |
| K$^{+}$ | $0.1035\times 10^{-1}$ | 0.6276 | -2.1871 |
| MgCl$^{+}$ | $0.9151\times 10^{-2}$ | 0.6717 | -2.2113 |
| NaSO$_{4}^{-}$ | $0.6381\times 10^{-2}$ | 0.6717 | -2.3679 |
| Ca$^{2+}$ | $0.5801\times 10^{-2}$ | 0.2465 | -2.8446 |
| MgSO$_{4}$ | $0.5774\times 10^{-2}$ | 1.0 | -2.2385 |
| CaCl$^{+}$ | $0.3686\times 10^{-2}$ | 0.6717 | -2.6063 |
| NaCl | $0.2775\times 10^{-2}$ | 1.0 | -2.5567 |
| HCO$_{3}^{-}$ | $0.1167\times 10^{-2}$ | 0.6906 | -3.0938 |
| CaSO$_{4}$ | $0.8114\times 10^{-3}$ | 1.0 | -3.0908 |
| NaHCO$_{3}$ | $0.3464\times 10^{-3}$ | 1.0 | -3.4463 |
| O$_{2}$(aq) | $0.2151\times 10^{-3}$ | 1.1734 | -3.5979 |
| KSO$_{4}^{-}$ | $0.1871\times 10^{-3}$ | 0.6717 | -3.9007 |
| MgHCO$_{3}^{+}$ | $0.1546\times 10^{-3}$ | 0.6717 | -3.9836 |
| SiO$_{2}$(aq) | $0.8536\times 10^{-4}$ | 1.1735 | -3.9993 |
| KCl | $0.5802\times 10^{-4}$ | 1.0 | -4.2364 |

### Minerals

The saturation indices of the equilibrium solution in [table:molalities_etc] are greater than 0 for a number of minerals, which suggests some minerals will precipitate.  Allowing this to occur, the `geochemistry` simulations and the GWB simulations predict that only dolomite and quartz actually precipitate (both codes produce the same precipitated mass) as shown in [table:minerals].

!table id=table:minerals caption=Calculated initial saturation indices, and the final mass of each precipitate in the stable phase assemblage
| Mineral | Initial SI | Amount formed (mg) |
| --- | --- | --- |
| Antigorite | 43 | 0 |
| Tremolite | 7.3 | 0 |
| Talc | 6.5 | 0 |
| Chrysotile | 4.5 | 0 |
| Sepiolite | 3.7 | 0 |
| Dolomite-ord | 3.3 | 0 |
| Dolomite | 3.3 | 50.63 |
| Anthophyllite | 3.0 | 0 |
| Huntite | 1.8 | 0 |
| Dolomite-dis | 1.8 | 0 |
| Magnesite | 0.95 | 0 |
| Calcite | 0.74 | 0 |
| Aragonite | 0.57 | 0 |
| Quartz | -0.01 | 1.028 |

!bibtex bibliography