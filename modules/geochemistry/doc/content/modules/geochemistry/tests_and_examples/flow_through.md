# Flow-through reactions that remove minerals

Chapter 13.3 of [!cite](bethke_2007) describes a "flow-through" process, whereby a mineral is removed from the system at each stage in a reaction process (such as progressively adding chemicals, changing the temperature, changing the pH, etc).  In the code, this is achieved by the following process, after each stage's equilibrium configuration is computed:

- subtracting $n_{k}$ from $M_{k}$;
- then setting $n_{k}$ to a tiny number (but not zero, otherwise it might be swapped out of the basis);
- setting the mole numbers of any surface components to zero, $M_{p}=0$, as well as the molalities of unoccupied surface sites, $m_{p}=0$ and adsorbed species, $m_{q}=0$.

Section 24.3 of [!cite](bethke_2007) describes an example of this involving the evaporation of seawater.  Note that [!cite](bethke_2007) uses the HMW activity model, but `geochemistry` uses the Debye-Huckel approach, so the MOOSE results are not identical to [!cite](bethke_2007).  Nevertheless, it is hoped that the following description helps users set up similar models.

The initial composition of the seawater is shown in [table:analysis].  In addition:

- CO$_{2}$(g) is used in the basis instead of H$^{+}$;
- the fugacity of CO$_{2}$(g) is fixed to $10^{-3.5}$;
- after equilibration, all minerals are [dumped](calcite_buffer.md) from the system.

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


### Case 1

To model evaporation, 996$\,$g of water is gradually removed from the solution, allowing minerals to precipitate, and then potentially dissolve, to maintain equilibrium.

### Case 2

To model evaporation, 996$\,$g of water is gradually removed from the solution, allowing minerals to precipitate, but when they precipitate they are immediately removed from the system following the "flow-through" method.

### Results

The results of case 1 are very different from case 2, as shown in Figures 24.7, 24.8 and 24.9 of [!cite](bethke_2007).

MOOSE gives the result ????





!bibtex bibliography