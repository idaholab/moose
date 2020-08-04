# Computing equilibrium activity ratios

This page follows Chapter 11 of [!cite](bethke_2007).

The `geochemistry` module can be used to provide activity ratios assuming equilibrium.

## Muscovite and kaolinite

Using the standard GWB database and the component kaolinite instead of Al$^{3+}$ (using a [swap](swap.md)), the reaction
\begin{equation}
\mathrm{muscovite} + 1.5\mathrm{H}_{2}\mathrm{O} + \mathrm{H}^{+} \rightleftharpoons \mathrm{K}^{+} + 1.5\mathrm{kaolinite} \ ,
\end{equation}
has an equilibrium constant $\log_{10}K = 3.42$ at 25$^{\circ}$C.  Assuming a water activity of 1, the equilibrium activity ratio can be computed to be
\begin{equation}
\frac{a_{\mathrm{K}^{+}}}{a_{\mathrm{H}^{+}}} = 10^{3.42} \ .
\end{equation}

To perform this calculation using the `geochemistry` module, a [GeochemicalModelDefinition](GeochemicalModelDefinition.md) object must be created with the desired mineral species:

!listing modules/geochemistry/test/tests/interrogate_reactions/muscovite.i block=UserObjects

Then a [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) must be created which specifies the desired swaps and the `interrogation = activity` instruction:

!listing modules/geochemistry/test/tests/interrogate_reactions/muscovite.i block=GeochemicalModelInterrogator

The final line of the output yields the desired information:

```
(A_K+)^1 (A_Al+++)^3 (A_SiO2(aq))^3 (A_H+)^-10 = 10^14.56
(A_K+)^1 (A_H+)^-1 = 10^3.421
```

## Muscovite and other minerals

The geochemical database can be manipulated to give the following reactions for muscovite:
\begin{equation}
\begin{aligned}
\mathrm{muscovite} & \rightleftharpoons -6\mathrm{quartz} +3\mathrm{maximum} + 2\mathrm{H}^{+} - 2\mathrm{K}^{+} & \log_{10}K=-9.681 \\
\mathrm{muscovite} & \rightleftharpoons -6\mathrm{tridymite} +3\mathrm{maximum} + 2\mathrm{H}^{+} - 2\mathrm{K}^{+} & \log_{10}K=-8.686 \\
\mathrm{muscovite} & \rightleftharpoons -6\mathrm{amrphsilica} +3\mathrm{maximum} + 2\mathrm{H}^{+} - 2\mathrm{K}^{+} & \log_{10}K= -1.967 \ 
\end{aligned}
\end{equation}
(at 25$^{\circ}$C).  These yield, respectively
\begin{equation}
\begin{aligned}
\frac{a_{\mathrm{K}^{+}}}{a_{\mathrm{H}^{+}}} & = 10^{4.84} \\
\frac{a_{\mathrm{K}^{+}}}{a_{\mathrm{H}^{+}}} & = 10^{4.34} \\
\frac{a_{\mathrm{K}^{+}}}{a_{\mathrm{H}^{+}}} & = 10^{0.98}
\end{aligned}
\end{equation}

To perform this calculation using the `geochemistry` module, a [GeochemicalModelDefinition](GeochemicalModelDefinition.md) object must be created with the desired mineral species:

!listing modules/geochemistry/test/tests/interrogate_reactions/muscovite2.i block=UserObjects

Then a [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) must be created which specifies the desired swaps and the `interrogation = activity` instruction:

!listing modules/geochemistry/test/tests/interrogate_reactions/muscovite2.i block=GeochemicalModelInterrogator

The output yields the desired information

```
(A_H2O)^6 (A_K+)^1 (A_Al+++)^3 (A_SiO2(aq))^3 (A_H+)^-10 = 10^14.56
(A_H2O)^6 (A_K+)^1 (A_Al+++)^3 (A_H+)^-10 = 10^26.56
(A_K+)^-2 (A_H+)^2 = 10^-9.681
(A_K+)^-2 (A_SiO2(aq))^-6 (A_H+)^2 = 10^14.31
(A_K+)^-2 (A_H+)^2 = 10^-8.686
(A_K+)^-2 (A_SiO2(aq))^-6 (A_H+)^2 = 10^14.31
(A_K+)^-2 (A_H+)^2 = 10^-1.967
```

## Ca-clinoptilolite, prehnite and quartz

Using the standard GWB database, prehnite instead of Al$^{3+}$, quartz instead of SiO$_{2}$(aq) (using [swaps](swap.md)), the reaction
\begin{equation}
\mathrm{clinoptilolite} + \mathrm{Ca}^{2+} \rightleftharpoons 6\mathrm{H}_{2}\mathrm{O} + \mathrm{prehnite} + 7\mathrm{quartz} + 2\mathrm{H}^{+} \ , 
\end{equation}
has an equilibrium constant $\log_{10}K = -10.23$ at 200$^{\circ}$C.  Assuming a water activity of 1, the equilibrium activity ratio can be computed to be
\begin{equation}
\frac{a_{\mathrm{Ca}^{2+}}}{a_{\mathrm{H}^{+}}^{2}} = 10^{-10.23} \ .
\end{equation}

To produce this result using the `geochemistry` module,  a [GeochemicalModelDefinition](GeochemicalModelDefinition.md) object must be created with the desired mineral species:

!listing modules/geochemistry/test/tests/interrogate_reactions/clinoptilolite2.i block=UserObjects

Then a [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) must be created which specifies the desired swaps and the `interrogation = activity` instruction:

!listing modules/geochemistry/test/tests/interrogate_reactions/clinoptilolite2.i block=GeochemicalModelInterrogator

The final line of the output yields the desired information:

```
(A_Ca++)^1 (A_Al+++)^2 (A_SiO2(aq))^10 (A_H+)^-8 = 10^-13.51
(A_Ca++)^-1 (A_SiO2(aq))^7 (A_H+)^2 = 10^-27.22
(A_Ca++)^-1 (A_H+)^2 = 10^-10.23
```


!bibtex bibliography