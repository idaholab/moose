# Computing pH and pe

This page follows Chapter 11 of [!cite](bethke_2007).

The `geochemistry` module can be used to provide values for pH and pe assuming equilibrium.  Recall that
\begin{equation}
\mathrm{pH} = -\log_{10}a_{\mathrm{H}^{+}} \ ,
\end{equation}
and
\begin{equation}
\mathrm{pe} = -\log_{10}a_{\mathrm{e}^{-}} \ ,
\end{equation}
where $a$ denotes activity.

An example of such a calculation involves the mineral hematite.  Hematite's equilibrium reaction is
\begin{equation}
\mathrm{hematite} \rightleftharpoons 2\mathrm{H}_{2}\mathrm{O} - 4\mathrm{H}^{+} + 2\mathrm{Fe}^{2+} + \frac{1}{2}\mathrm{O}_{2}\mathrm{(aq)} \ ,
\end{equation}
has an equilibrium constant $\log_{10}K = -16.9$ at 25$^{\circ}$C.  Assuming a water activity of 1, and $a_{\mathrm{Fe}^{2+}} = 10^{-10}$, equilibrium reads
\begin{equation}
\label{eqn:ph}
\log_{10}a_{\mathrm{O}_{2}\mathrm{(aq)}} = 6.17 - 8\mathrm{pH} \ .
\end{equation}
Introducing e$^{-}$ into the basis in favour of O$_{2}$(aq) yields
\begin{equation}
\label{eqn:pe}
\mathrm{pe} = 23.03 - 3\mathrm{pH} \ .
\end{equation}

To perform this calculation using the `geochemistry` module, a [GeochemicalModelDefinition](GeochemicalModelDefinition.md) object must be created with the desired mineral species:

!listing modules/geochemistry/test/tests/interrogate_reactions/hematite.i block=UserObjects

Then a [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) must be created which specifies the desired swaps and the `interrogation = activity` instruction:

!listing modules/geochemistry/test/tests/interrogate_reactions/hematite.i block=GeochemicalModelInterrogator

The output yields the desired information:

```
(A_H+)^-4 (A_O2(aq))^0.5 = 10^3.068
(A_H+)^-6 (A_e-)^-2 = 10^46.07
```

The square of first equation is [eqn:ph], while the square-root of the second yields [eqn:pe].


!bibtex bibliography