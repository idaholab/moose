# Saturation indieces of halite and anhyrite at Sebkhat El Melah

This example closely follows Section 8.4 of [!cite](bethke_2007).

The saturation indices of halite (NaCl) and anhydrite (CaSO$_{4}$) are computed for brine samples from Sebkhat El Melah.  The composition of the brine is shown in [table:analysis].  In addition:

- the pH is 7.15;
- charge balance is performed on Cl$^{-}$.

!table id=table:analysis caption=Major element composition of brine in the RZ-2 well at Sebkhat El Melah
| Species | Concentration (g.l$^{-1}$) |
| --- | --- |
| Cl$^{-}$ | 195 |
| Mg$^{2+}$ | 52.1 |
| Na$^{+}$ | 43.1 |
| SO$_{4}^{2-}$ | 27.4 |
| K$^{+}$ | 6.90 |
| Br$^{-}$ | 2.25 |
| Ca$^{2+}$ | 0.20 |
| HCO$_{3}^{-}$ | 0.14 |

## MOOSE input file

The MOOSE input file contains the usual [GeochemicalModelDefinition](GeochemicalModelDefinition.md) that specifies the database file to use, the basis species and the minerals of interest.  The flag `piecewise_linear_interpolation = true` in order to compare with the [Geochemists Workbench](https://www.gwb.com/) result

!listing modules/geochemistry/test/tests/solubilities_and_activities/sebkhat_el_melah.i block=UserObjects

To instruct MOOSE to find the equilibrium configuration, a [TimeIndependentReactionSolver](actions/AddTimeIndependentReactionSolverAction.md) is used:

- The bulk mole number of the aqueous species is also appropriately.  The numbers are different than the concentration in g.l$^{-1}$ given in the above table, and may be worked out using the [TDS](tests_and_examples/ic_unit_conversions.md).
- The pH is set by providing the relevant activity for H$^{+}$.
- The `prevent_precipitation` input prevents any minerals from precipitating when finding the equilibrium configuration, even if their saturation indices are positive.
- The `max_ionic_strength` is set large in this case in order to match that specified by [!cite](bethke_2007) for this example.  [!cite](bethke_2007) notes that the Debye-Huckel theory is not valid above ionic strengths of about 3$\,$molal, and uses this example to demonstrate the resulting large errors when compared with experiment.  In this page, the comparison with experiment is not performed: we are just using this example to demonstrate agreement with the [Geochemists Workbench](https://www.gwb.com/) software.
- The other flags enable an accurate comparison with the [Geochemists Workbench](https://www.gwb.com/) software.

!listing modules/geochemistry/test/tests/solubilities_and_activities/sebkhat_el_melah.i block=TimeIndependentReactionSolver

## Geochemists Workbench input file

The [Geochemists Workbench](https://www.gwb.com/) input file for the precipitation case is:

!listing modules/geochemistry/test/tests/solubilities_and_activities/sebkhat_el_melah.rea

## Results

The `geochemistry` module and the [Geochemists Workbench](https://www.gwb.com/) both predict identical reuslts, in agreement with [!cite](bethke_2007):

- the saturation index of halite is approximately -0.94
- the saturation index of anhydrite is approximately -1.7.

!bibtex bibliography