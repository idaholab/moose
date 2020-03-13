# Dissolution of pyrite, with and without fixed oxygen fugacity

Section 14.2 of [!cite](bethke_2007) and involves studying the dissolution of pyrite, with and without fixed oxygen fugacity.

The initial composition of the water is shown in [table:composition].  In addition:

- hematite is used in place of Fe$^{2+}$ and its free concentration is 1$\,$mg (per 1$\,$kg of solvent water);
- O$_{2}$(g) is used in place of O$_{2}$(aq), and its initial fugacity is set to $0.2$;
- the initial pH is 6.5.

!table id=table:composition caption=Initial bulk composition
| Species | Concentration (mg.kg$^{-1}$) |
| --- | --- |
| Ca$^{2+}$ | 4 |
| Mg$^{2+}$ | 1 |
| Na$^{+}$ | 2 |
| HCO$_{3}^{-}$ | 18 |
| SO$_{4}^{2-}$ | 3 |
| Cl$^{-}$ | 5 |
| Hematite (in place of Fe$^{2+}$) | 1 mg (free) |


### Case 1

10$\,$mg of pyrite is slowly added to the system, without allowing any oxygen to enter or exit the system.

### Case 2

1000$\,$mg of pyrite is slowly added to the system, with fixed fugacity of oxygen $f_{\mathrm{O}_{2}}=0.2$.


### Results

[!cite](bethke_2007) predicts that the two cases are dramatically different (see [!cite](bethke_2007) Figure 14.2 to 14.5).  In Case 1, about 8$\,$mg of pyrite dissolves, precipitating hematite, before hematite suddenly dissolves and no further pyrite dissolution occurs.  In Case 2, pyrite dissolution and hematite precipitation continues indefinitely.

MOOSE produces the result ????

!bibtex bibliography