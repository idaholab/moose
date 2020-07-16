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

  - SO4-- 0.3963E-3 mol ($\approx 35\,$mg.kg$^{-1}$)
  - Ca++ 0.407E-3 mol ($\approx 15\,$mg.kg$^{-1}$)
  - Cl- 0.46E-3 mol ($\approx 15\,$mg.kg$^{-1}$ this is the charge-balance species)
  - Na+ 0.473E-3 mol ($\approx 10\,$mg.kg$^{-1}$)
  - Mg++ 0.0895E-3 mol ($\approx 2\,$mg.kg$^{-1}$)
  - NO3- 3.508E-5 mol ($\approx 2\,$mg.kg$^{-1}$ )
  - CH4(aq) 2.712E-5 mol ($\approx 0.4\,$mg.kg$^{-1}$)
  - NO2- 9.456E-6 mol ($\approx 0.4\,$mg.kg$^{-1}$)
  - CH3COO- 5.5526E-6 mol ($\approx 0.3\,$mg.kg$^{-1}$)
  - Fe++ 3.895E-6 mol ($\approx 0.2\,$mg.kg$^{-1}$)
  - NH4+ 6.029E-6 mol ($\approx 0.1\,$mg.kg$^{-1}$)
  - HS- 1.6445E-6 mol ($\approx 0.05\,$mg.kg$^{-1}$)

 - The free composition of the remaining basis species are:

  - O2(aq) 3.399E-6 free molal ($\approx 0.1\,$mg.kg$^{-1}$ free)
  - H2(aq) 0.004E-6 free molal ($\approx 0.004\times 10^{-6}\,$mol.kg$^{-1}$, free (not bulk))

- All mineralisation reactions are ignored.

## MOOSE input file

The MOOSE input file that corresponding to this model description is

!listing modules/geochemistry/test/tests/redox_disequilibrium/microbial.i

## GWB 

The [Geochemists Workbench](https://www.gwb.com/) input file corresponding to this model is

!listing modules/geochemistry/test/tests/redox_disequilibrium/microbial.rea

## Nernst potentials

[!cite](bethke_2007) computes the Nernst potentials for each redox couple as shown in [table:nernst]

!table id=table:nernst caption=Nernst potentials for redox couple
| Reaction | Eh (mV) |
| --- | --- |
| e$^{-}$ + $\frac{1}{4}$O$_{2}$(aq) + H$^{+}$ $\rightleftharpoons$ $\frac{1}{2}$H$_{2}$O | 836 |
| 2e$^{-}$ + 2H$^{+}$ + NO$_{3}^{-}$ $\rightleftharpoons$ H$_{2}$O + NO$_{2}^{-}$ | 481 |
| 8e$^{-}$ + 10H$^{+}$ + NO$_{3}^{-}$ $\rightleftharpoons$ 3H$_{2}$O + NH$_{4}^{+}$ | 443 |
| 6e$^{-}$ + 8H$^{+}$ + NO$_{2}^{-}$ $\rightleftharpoons$ 2H$_{2}$O + NH$_{4}^{+}$ | 430 |
| e$^{-}$ + Fe$^{3+}$ $\rightleftharpoons$ Fe$^{2+}$ | 321 |
| 8e$^{-}$ + 9H$^{+}$ + SO$_{4}^{2-}$ $\rightleftharpoons$ 4H$_{2}$O + HS$^{-}$ | -126 |
| 4e$^{-}$ + $\frac{9}{2}$H$^{+}$ + $\frac{1}{2}$CH$_{3}$COO$^{-}$ $\rightleftharpoons$ H$_{2}$O + CH$_{4}$(aq) | -145 |
| 8e$^{-}$ + 9H$^{+}$ + HCO$_{3}^{-}$ $\rightleftharpoons$ 3H$_{2}$O + CH$_{4}$(aq) | -188 |
| 2e$^{-}$ + 2H$^{+}$ $\rightleftharpoons$ H$_{2}$(aq) | -199 |
| 8e$^{-}$ + 9H$^{+}$ + 2HCO$_{3}^{-}$ $\rightleftharpoons$ 4H$_{2}$O + CH$_{3}$COO$^{-}$ | -230 |

This output is produced by the [Geochemists Workbench](https://www.gwb.com/) software.  The output produced by `geochemistry` looks a little different:

```
Nernst potentials:
e- = 0.5*H2O - 1*H+ - 0.25*O2(aq);  Eh = 0.8361V
e- = 0.5*H2O - 1*H+ + 0.5*NO2- - 0.5*NO3-;  Eh = 0.4809V
e- = 0.375*H2O - 1.25*H+ + 0.125*NH4+ - 0.125*NO3-;  Eh = 0.4425V
e- = 1*Fe++ - 1*Fe+++;  Eh = 0.3206V
e- = 0.5*H2O - 1.125*H+ - 0.125*SO4-- + 0.125*HS-;  Eh = -0.1263V
e- = 0.375*H2O - 1.125*H+ - 0.125*HCO3- + 0.125*CH4(aq);  Eh = -0.1875V
e- = -1*H+ + 0.5*H2(aq);  Eh = -0.1986V
e- = 0.5*H2O - 1.125*H+ - 0.25*HCO3- + 0.125*CH3COO-;  Eh = -0.2298V
```

The differences are:

- the reactions are all written with "e-" on the left-hand side
- the values for Eh sometimes differ in their fourth significant figure (this is presumably due to `geochemistry` using higher precision than GWB)
- the following reactions are missing:

  - 6e$^{-}$ + 8H$^{+}$ + NO$_{2}^{-}$ $\rightleftharpoons$ 2H$_{2}$O + NH$_{4}^{+}$ (Eh = 430) and 
  - 4e$^{-}$ + $\frac{9}{2}$H$^{+}$ + $\frac{1}{2}$CH$_{3}$COO$^{-}$ $\rightleftharpoons$ H$_{2}$O + CH$_{4}$(aq) (Eh = -145)

The reason these reactions do not appear is that `geochemistry` outputs a minimal set only, from which others can be derived.  For instance, the difference of the two equations
\begin{equation}
\begin{aligned}
4\mathrm{e}^{-} & = \frac{3}{2}\mathrm{H}_{2}\mathrm{O} - 5\mathrm{H}^{+} + \frac{1}{2}\mathrm{NH}_{4}^{+} - \frac{1}{2}\mathrm{NO}_{3}^{-} & \mathrm{Eh} = 4\times 442.5\,\mathrm{mV}\\
\mathrm{e}^{-} & = \frac{1}{2}\mathrm{H}_{2}\mathrm{O} - \mathrm{H}^{+} + \frac{1}{2}\mathrm{NO}_{2}^{-} - \frac{1}{2}\mathrm{NO}_{3}^{-} & \mathrm{Eh} = 480.9\,\mathrm{mV}\\
\end{aligned}
\end{equation}
produces the equation
\begin{equation}
3\mathrm{e}^{-} = \mathrm{H}_{2}\mathrm{O} - 4\mathrm{H}^{+} + \frac{1}{2}\mathrm{NH}_{4}^{+} - \frac{1}{2}\mathrm{NO}_{2}^{-} \ \ \ \   \mathrm{Eh} = 1289.1\,\mathrm{mV}
\end{equation}
which produces the result $\mathrm{Eh} = 430\,$mV when divided by 3.  (Remember that Eh values quoted by GWB are normalised to the number of electrons, but when manipulating equations in the way just described, the number of electrons must be accounted for.)

!bibtex bibliography