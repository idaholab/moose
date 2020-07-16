# Computing equilibrium temperature or activity

This page follows Chapter 11 of [!cite](bethke_2007).

The `geochemistry` module can be used to provide equilibrium temperature or activity.  For instance, using the standard GWB database and anydrite instead of Ca$^{2+}$ as a component (using a [swap](swap.md)), the reaction
\begin{equation}
\mathrm{gypsum}\rightleftharpoons \mathrm{anhydrite} + 2\mathrm{H}_{2}\mathrm{O} \ ,
\end{equation}
has the equilibrium equation
\begin{equation}
\log_{10}K = 2\log_{10}a_{\mathrm{H}_{2}\mathrm{O}} \ .
\end{equation}
Then:

- If water activity is 1, the equilibrium temperature (where $K=1$) is found to be 43.7$^{\circ}$C.
- If water activity is 0.7, the equilibrium temperature (where $K=1$) is found to be 11.7$^{\circ}$C.
- If the temperature is 25$^{\circ}$C, the equilibrium is attained when water activity is 0.815.

To perform this calculation using the `geochemistry` module, a [GeochemicalModelDefinition](GeochemicalModelDefinition.md) object must be created with the desired mineral species:

!listing modules/geochemistry/test/tests/interrogate_reactions/gypsum.i block=UserObjects

Then a [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) must be created which specifies the desired swaps and the `interrogation = eqm_temperature` instruction:

!listing modules/geochemistry/test/tests/interrogate_reactions/gypsum.i block=GeochemicalModelInterrogator

The output yields the desired information:

```
Not enough activites known to compute equilibrium temperature for reaction
  Gypsum = 2\*H2O \+ 1\*Ca\+\+ \+ 1\*SO4--  .  log10\(K\) = -4.451
Gypsum.  T = 43.6625degC
```

The first lines state that equilibrium temperature for the reaction
\begin{equation}
\mathrm{gypsum} \rightleftharpoons = 2\mathrm{H}_{2}\mathrm{O} + \mathrm{Ca}^{2+} + \mathrm{SO}_{4}^{2-} \ ,
\end{equation}
cannot be found because the activities of Ca$^{2+}$ and SO$_{4}^{2-}$ are unknown.  However, when Anhydrite is put into the basis in place of Ca$^{2+}$, the equilibrium temperature can be found, and the result is found on the final line.

Altering the activity value for H$_{2}$O using the `activity_values` input enables finding the equilibrium temperature for different $a_{\mathrm{H}_{2}\mathrm{O}}$.

To find the activity of water given $T=25^{\circ}$C, simply use:

- `interrogation = activity`
- remove the `activity_species` and `activity_values` lines

as in the page on [equilibrium activity ratios](activity_ratios.md).  This produces

```
(A_H2O)^2 (A_Ca++)^1 (A_SO4--)^1 = 10^-4.451
(A_H2O)^2 = 10^-0.1775
```

where the last line is the desired result: $a_{\mathrm{H}_{2}\mathrm{O}} = 10^{-0.1775/2} = 0.815$.


!bibtex bibliography