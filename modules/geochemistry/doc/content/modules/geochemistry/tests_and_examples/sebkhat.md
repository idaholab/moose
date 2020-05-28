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

Using the Debye-Huckel-B-dot formulae, [!cite](bethke_2007) predicts that:

- the saturation index of halite is approximately -1;
- the saturation index of anhydrite is approximately -1.7.

The MOOSE results are ????

!listing modules/geochemistry/test/tests/solubilities_and_activities/sebkhat_el_melah.i

!bibtex bibliography