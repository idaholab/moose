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
with $\log_{10}K = -5.48$.  MOOSE produces this result ????

## Pyrite

The reaction for the mineral Pyrite is written in the database as
\begin{equation}
\mathrm{pyrite} \rightleftharpoons -\mathrm{H}_{2}\mathrm{O} + \mathrm{Fe}^{2+} + 1.75\mathrm{HS}^{-} + 0.25\mathrm{SO}^_{4}^{2-} + 0.25\mathrm{H}^{+} \ ,
\end{equation}
with an equilibrium constant of $\log_{10}K = -24.7025$ at 25$^{\circ}$C.  Using [basis-swaps](swap.md), it can be expressed in terms of O$_{2}$(aq) (instead of HS$^{-}$) and Fe(OH)$_{3}$(ppd) (instead of Fe$^{2+}$):
\begin{equation}
\mathrm{pyrite} \rightleftharpoons - \frac{7}{2}\mathrm{H}_{2}\mathrm{O} + \frac{15}{4}\mathrm{O}_{2}\mathrm{(aq)} + \mathrm{Fe(OH)}_{3}\mathrm{(ppd)} + 2\mathrm{SO}$_{4}^{2-} + 4\mathrm{H}^{+} \ .
\end{equation}
with $\log_{10}K = -5.48$.




!bibtex bibliography