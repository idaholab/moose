# Reaction balancing

This page follows Chapter 11 of [!cite](bethke_2007).

The `geochemistry` module can be used to provide balanced reactions in terms of user-defined components.

## Ca-clinoptilolite

The reaction for the mineral Ca-clinoptilolite is written in the database as
\begin{equation}
\mathrm{ClinoptilCa} \rightleftharpoons 12\mathrm{H}_{2}\mathrm{O} + \mathrm{Ca}^{2+} + 2\mathrm{Al}^{3+} + 10\mathrm{SiO}_{2}\mathrm{(aq)} - 8\mathrm{H}^{+} \ ,
\end{equation}
with an equilibrium constant of $\log_{10}K = -9.12$ at 25$^{\circ}$C.  Using [basis-swaps](swap.md), it can be expressed in terms of muscovite (instead of Al$^{3+}$), quartz (instead of SiO$_{2}$(aq)) and OH${-}$ instead of H$^{+}$:
\begin{equation}
\mathrm{ClinoptilCa} \rightleftharpoons -\frac{2}{3}\mathrm{K}^{+} + \frac{20}{3}\mathrm{H}_{2}\mathrm{O} + \mathrm{Ca}^{2+} + \frac{2}{3}\mathrm{muscovite} + 8\mathrm{quartz} + \frac{4}{3}\mathrm{OH}^{-} \ ,
\end{equation}
with $\log_{10}K = -5.48$.

To perform this calculation using the `geochemistry` module, a [GeochemicalModelDefinition](GeochemicalModelDefinition.md) object must be created with the desired mineral species:

!listing modules/geochemistry/test/tests/interrogate_reactions/clinoptilolite.i block=UserObjects

Then a [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) must be created which specifies the desired swaps:

!listing modules/geochemistry/test/tests/interrogate_reactions/clinoptilolite.i block=GeochemicalModelInterrogator

The first and last lines of the output yield the desired information:

```
Clinoptil-Ca = 12*H2O + 1*Ca++ + 2*Al+++ + 10*SiO2(aq) - 8*H+  .  log10(K) = -9.12
Clinoptil-Ca = 8*H2O + 1*Ca++ + 0.6667*Muscovite + 8*SiO2(aq) - 1.333*H+ - 0.6667*K+  .  log10(K) = -18.83
Clinoptil-Ca = 8*H2O + 1*Ca++ + 0.6667*Muscovite + 8*Quartz - 1.333*H+ - 0.6667*K+  .  log10(K) = 13.16
Clinoptil-Ca = 6.667*H2O + 1*Ca++ + 0.6667*Muscovite + 8*Quartz + 1.333*OH- - 0.6667*K+  .  log10(K) = -5.484
```


## Pyrite

The standard reaction for the mineral Pyrite is
\begin{equation}
\mathrm{pyrite} \rightleftharpoons -\mathrm{H}_{2}\mathrm{O} + \mathrm{Fe}^{2+} - \frac{7}{2}\mathrm{O}_{2}\mathrm{(aq)} + 2\mathrm{SO}_{4}^{2-} + 2\mathrm{H}^{+} \ ,
\end{equation}
Using [basis-swaps](swap.md), it can be expressed in terms Fe(OH)$_{3}$(ppd) (instead of Fe$^{2+}$):
\begin{equation}
\mathrm{pyrite} \rightleftharpoons - \frac{7}{2}\mathrm{H}_{2}\mathrm{O} - \frac{15}{4}\mathrm{O}_{2}\mathrm{(aq)} + \mathrm{Fe(OH)}_{3}\mathrm{(ppd)} + 2\mathrm{SO}_{4}^{2-} + 4\mathrm{H}^{+} \ .
\end{equation}
Alternatively, it can be expressed in terms of H$_{2}$S(aq) instead of SO$_{4}^{-}$:
\begin{equation}
\mathrm{pyrite} \rightleftharpoons - \mathrm{H}_{2}\mathrm{O} + \frac{1}{2}\mathrm{O}_{2}\mathrm{(aq)} + \mathrm{Fe}^{++} + 2\mathrm{H}_{2}\mathrm{S(aq)} - 2\mathrm{H}^{+} \ .
\end{equation}
Or in terms of H$_{2}$S(aq) and SO$_{4}^{-}$:
\begin{equation}
\mathrm{pyrite} \rightleftharpoons - \mathrm{H}_{2}\mathrm{O} + \frac{7}{4}\mathrm{H}_{2}\mathrm{S(aq)} + \mathrm{Fe}^{++} + \frac{1}{4}\mathrm{SO}_{4}^{2-} - \frac{3}{2}\mathrm{H}^{+} \ .
\end{equation}


To perform this calculation using the `geochemistry` module, a [GeochemicalModelDefinition](GeochemicalModelDefinition.md) object must be created with the desired mineral species:

!listing modules/geochemistry/test/tests/interrogate_reactions/pyrite.i block=UserObjects

Then a [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) must be created which specifies the desired swaps:

!listing modules/geochemistry/test/tests/interrogate_reactions/pyrite.i block=GeochemicalModelInterrogator

The output yields the desired information:

```
Pyrite = -1*H2O + 1*Fe++ + 2*SO4-- + 2*H+ - 3.5*O2(aq)  .  log10(K) = 217.4
Pyrite = -3.5*H2O + 1*Fe(OH)3(ppd) + 2*SO4-- + 4*H+ - 3.75*O2(aq)  .  log10(K) = 221
Pyrite = -1*H2O + 1*Fe++ + 2*SO4-- + 2*H+ - 3.5*O2(aq)  .  log10(K) = 217.4
Pyrite = -1*H2O + 1*Fe++ + 2*H2S(aq) - 2*H+ + 0.5*O2(aq)  .  log10(K) = -45.39
Pyrite = -1*H2O + 1*Fe++ + 1.75*H2S(aq) - 1.5*H+ + 0.25*SO4--  .  log10(K) = -12.54
```

!bibtex bibliography