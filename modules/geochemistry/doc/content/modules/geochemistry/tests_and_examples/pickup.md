# Adding fluids with different temperatures

Section 22.2 of [!cite](bethke_2007) describes the mixing of different fluids, which Bethke terms as "pickup".

## Step 1

The simulation begins with forming an equilibrium solution of seawater.  The major element composition is shown in [table:seawater].  In addition:

- $T=4^{\circ}$C;
- pH $=8.1$;
- the following minerals are prevented from precipitating: Quartz, Tridymite, Cristobalite, Chalcedony and Hematite.

The system is equilibrated, and the final composition saved (I'm not sure whether this includes any precipitated minerals or not).


!table id=table:seawater caption=Major element composition of seawater
| Species | Concentration (mmolal) |
| --- | --- |
| Cl$^{-}$ | 559 |
| Na$^{+}$ | 480 |
| Mg$^{2+}$ | 54.5 |
| SO$_{4}^{2-}$ | 29.5 |
| Ca$^{2+}$ | 10.5 |
| K$^{+}$ | 10.1 |
| HCO$_{3}^{-}$ | 2.4 |
| Ba$^{2+}$ | 0.20 |
| SiO$_{2}$(aq) | 0.17 |
| Sr$^{2+}$ | 0.09 |
| Zn$^{2+}$ | 0.01 |
| Cu$^{+}$ | 0.007 |
| Al$^{3+}$ | 0.005 |
| Fe$^{2+}$ | 0.001 |
| Mn$^{2+}$ | 0.001 |
| O$_{2}$(aq) | 123 (free) |

## Step 2

The hot hydrothermal fluid has composition shown in [table:hydrothermal].  In addition:

- the temperature (I think) is 273$^{\circ}$C;
- the pH is 4;
- H$_{s}$S(aq) is used in the basis instead of O$_{2}$(aq).

The system is brought to equilibrium and then all precipitated minerals are [dumped](calcite_buffer.md).

!table id=table:hydrothermal caption=Major element composition of hydrothermal fluid
| Species | Concentration (mmolal) |
| --- | --- |
| Cl$^{-}$ | 600 |
| Na$^{+}$ | 529 |
| Mg$^{2+}$ | 0.01 |
| SO$_{4}^{2-}$ | 0.01 |
| Ca$^{2+}$ | 21.6 |
| K$^{+}$ | 26.7 |
| HCO$_{3}^{-}$ | 2.0 |
| Ba$^{2+}$ | 15 |
| SiO$_{2}$(aq) | 20.2 |
| Sr$^{2+}$ | 100.5 |
| Zn$^{2+}$ | 41 |
| Cu$^{+}$ | 0.02 |
| Al$^{3+}$ | 4.1 |
| Fe$^{2+}$ | 903 |
| Mn$^{2+}$ | 1039 |
| H$_{2}$S(aq) | 6.81 |

## Step 3

The seawater at 4$^{\circ}$C is slowly mixed into the geothermal fluid at 273$^{\circ}$C until the final ratio is 10(seawater):1(geothermal).  A constant heat capacity is assumed.

## Results

Figures 22.3 and 22.4 of [!cite](bethke_2007) show the results.

MOOSE produces the result ????


!bibtex bibliography