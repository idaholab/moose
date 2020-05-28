# Energy available for microbial respiration

This example closely follows Section 7.4 of [!cite](bethke_2007) in which the energy available for microbial respiration is computed using the Nernst Eh values of various redox half-reactions.  [!cite](bethke_2007) has several pages of interesting explanation that are not repeated here: we are focussing primarily on the MOOSE model and validating the MOOSE results.

The following assumptions are made.

- The following alternate oxidation states are in redox disequilibrium:

  - CH3COO-
  - CH4(aq)
  - Fe+++
  - H2(aq)
  - HS-
  - NH4+
  - NO2-
- The pH is 6
- The fugacity of CO$_{2}$(g) is fixed at 0.01, and it is used in the basis in place of the aqueous species HCO$_{3}^{-}$
- There is 1 free cm$^{3}$ of the mineral Fe(OH)$_{3}$(ppd) and that it is used in the basis in place of the redox aqueous species Fe$^{3+}$.
- The bulk composition of the other basis species are:

  - SO4-- $=35\,$mg.kg$^{-1}$
  - Ca++ $=15\,$mg.kg$^{-1}$
  - Cl- $=15\,$mg.kg$^{-1}$
  - Na+ $=10\,$mg.kg$^{-1}$
  - Mg++ $=2\,$mg.kg$^{-1}$
  - NO3- $=2\,$mg.kg$^{-1}$
  - CH4(aq) $=0.4\,$mg.kg$^{-1}$
  - NO2- $=0.4\,$mg.kg$^{-1}$
  - CH3COO- $=0.3\,$mg.kg$^{-1}$
  - Fe++ $=0.2\,$mg.kg$^{-1}$
  - NH4+ $=0.1\,$mg.kg$^{-1}$
  - HS- $=0.05\,$mg.kg$^{-1}$
- The free composition of the remaining basis species are:

  - O2(aq) $=0.1\,$mg.kg$^{-1}$, free
  - H2(aq) $=0.004\times 10^{-6}\,$mol.kg$^{-1}$, free (not bulk)
- All mineralisation reactions are ignored.
  

## Nernst potentials

[!cite](bethke_2007) computes the Nernst potentials for each redox couple as shown in [table:nernst].  These may be compared with the prediction from MOOSE in ????

!table id=table:nernst caption=Nernst potentials for redox couple
| Reaction | Eh (mV) |
| --- | --- |
| e$^{-}$ + $\frac{1}{4}$O$_{2}$(aq) + H$^{+}$ $\rightleftharpoons$ $\frac{1}{2}$H$_{2}$O | 836 |
| 2e$^{-}$ + 2H$^{+}$ + NO$_{3}^{-}$ $\rightleftharpoons$ H$_{2}$O + NO$_{2}^{-}$ | 481 |
| 8e$^{-}$ + 10H$^{+}$ + NO$_{3}^{-}$ $\rightleftharpoons$ 3H$_{2}$O + NH$_{4}^{+}$ | 443 |
| 6e$^{-}$ + 8H$^{+}$ + NO$_{2}^{-}$ $\rightleftharpoons$ 2H$_{2}$O + NH$_{4}^{+}$ | 430 |
| e$^{-}$ + Fe$^{3+}$ $\rightleftharpoons$ Fe$^{2+}$ | 322 |
| 8e$^{-}$ + 9H$^{+}$ + SO$_{4}^{2-}$ $\rightleftharpoons$ 4H$_{2}$O + HS$^{-}$ | -126 |
| 4e$^{-}$ + $\frac{9}{2}$H$^{+}$ + $\frac{1}{2}$CH$_{3}$COO$^{-}$ $\rightleftharpoons$ H$_{2}$O + CH$_{4}$(aq) | -145 |
| 8e$^{-}$ + 9H$^{+}$ + HCO$_{3}^{-}$ $\rightleftharpoons$ 3H$_{2}$O + CH$_{4}$(aq) | -187 |
| 2e$^{-}$ + 2H$^{+}$ $\rightleftharpoons$ H$_{2}$(aq) | -197 |
| 8e$^{-}$ + 9H$^{+}$ + 2HCO$_{3}^{-}$ $\rightleftharpoons$ 4H$_{2}$O + CH$_{3}$COO$^{-}$ | -230 |

!listing modules/geochemistry/test/tests/redox_disequilibrium/microbial.i

!bibtex bibliography