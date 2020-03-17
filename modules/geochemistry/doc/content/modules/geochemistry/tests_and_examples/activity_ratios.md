# Computing equilibrium activity ratios

This page follows Chapter 11 of [!cite](bethke_2007).

The `geochemistry` module can be used to provide activity ratios assuming equilibrium

## Muscovite and kaolinite

Using the standard GWB database and the component kaolinite instead of Al$^{3+}$ (using a [swap](swap.md)), the reaction
\begin{equation}
\mathrm{muscovite} + 1.5\mathrm{H}_{2}\mathrm{O} + \mathrm{H}^{+} \rightleftharpoons \mathrm{K}^{+} + 1.5\mathrm{kaolinite} \ ,
\end{equation}
has an equilibrium constant $\log_{10}K = 3.42$ at 25$^{\circ}$C.  Assuming a water activity of 1, the equilibrium activity ratio can be computed to be
\begin{equation}
\frac{a_{\mathrm{K}^{+}}}{a_{\mathrm{H}^{+}}} = 10^{3.42} \ .
\end{equation}
MOOSE produces the result ????

## Ca-clinoptilolite, prehnite and quartz

Using the standard GWB database, prehnite instead of Al$^{3+}$, quartz instead of SiO$_{2}$(aq) (using [swaps](swap.md)), the reaction
\begin{equation}
\mathrm{clinoptilolite} + \mathrm{Ca}^{2+} \rightleftharpoons 6\mathrm{H}_{2}\mathrm{O} + \mathrm{prehnite} + 7\mathrm{quartz} + 2\mathrm{H}^{+} \ , 
\end{equation}
has an equilibrium constant $\log_{10}K = -10.23$ at 200$^{\circ}$C.  Assuming a water activity of 1, the equilibrium activity ratio can be computed to be
\begin{equation}
\frac{a_{\mathrm{Ca}^{2+}}}{a_{\mathrm{H}^{+}}^{2}} = 10^{-10.23} \ .
\end{equation}
MOOSE produces the result ????


!bibtex bibliography