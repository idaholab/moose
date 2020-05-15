# Solubility of gypsum and associated activities

This example closely follows Section 8.3 of [!cite](bethke_2007).

The solubility of gypsum (CaSO$_{4}$.2H$_{2}$O) as a function of NaCl concentration is explored.  The following assumptions are made:

- The equilibrium constant for gypsum dissolution is $10^{-4.5805}$.
- Gypsum is used in the basis instead of the aqueous species Ca$^{2+}$.
- The amount of free gypsum is 100$\,$g.
- Charge balance is performed on SO$_{4}^{2-}$.

A simulation is then run with different molal quantities of Na$^{+}$ and Cl$^{-}$.  Hence, the basis is (H20, Na+, Cl-, SO4--, gypsum).  The algorithm predicts how much bulk gypsum is needed to maintain 100$\,$g of free gypsum, since some of it dissolves to become Ca$^{2+}$, SO$_{4}^{2-}$, as well as CaCl$^{+}$, etc.  From the concentrations of the secondary aqueous species, the solubility of gypsum can be computed.  The following are recorded, as a function of NaCl molality:

- solubility of gypsum;
- molality of CaCl$^{+}$, NaSO$_{4}^{-}$, SO$_{4}^{2-}$, CaSO$_{4}$, Ca$^{2+}$;
- activity of CaCl$^{+}$, NaSO$_{4}^{-}$, SO$_{4}^{2-}$, CaSO$_{4}$, Ca$^{2+}$.

The results are shown in Figures 8.6 and 8.7 of [!cite](bethke_2007).  These can be compared with the MOOSE results shown in ????

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.i

!bibtex bibliography