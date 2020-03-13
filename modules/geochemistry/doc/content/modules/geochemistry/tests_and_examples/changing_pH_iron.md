# Exploring the impact of pH on sorption

Section 14.3 of [!cite](bethke_2007) describes how to explore the impact of pH on sorption.

This page builds upon the [surface complexation example](surface_complexation.md).  It is assumed that there are two sorbing sites:
\begin{equation}
A_{p}=\left(\ \mathrm{>(s)FeOH},\  \mathrm{>(w)FeOH}\,\right) \ .
\end{equation}
[!cite](bethke_2007) uses the convention that a ">" indicates something to do with sorption.  The sorbing reactions, written in the [documentation on equilibrium reactions](equilibrium.md) as
\begin{equation}
A_{q} \rightleftharpoons \nu_{wq}A_{w} + \sum_{i}\nu_{iq}A_{i} + \sum_{k}\nu_{kq}A_{k} + \sum_{m}\nu_{mq}A_{m} + \nu_{pq}A_{p} \ ,
\end{equation}
are, specifically:
\begin{equation}
\begin{aligned}
\mathrm{>(w)FeOH}_{2}^{+} & \rightleftharpoons \mathrm{>(w)FeOH} + \mathrm{H}^{+} & \log_{10}K=-7.29\ , \\
\mathrm{>(w)FeO}^{-} & \rightleftharpoons \mathrm{>(w)FeOH} - \mathrm{H}^{+} & \log_{10}K=8.93\ , \\
\mathrm{>(s)FeOH}_{2}^{+} & \rightleftharpoons \mathrm{>(s)FeOH} + \mathrm{H}^{+} & \log_{10}K=-7.29\ , \\
\mathrm{>(s)FeO}^{-} & \rightleftharpoons \mathrm{>(s)FeOH} - \mathrm{H}^{+} & \log_{10}K=8.93\ . \\
\end{aligned}
\end{equation}
The sorption occurs on the ferric hydroxide mineral, Fe(OH)$_{3}$, called Fe(OH)3(ppd) in the database.  This is used in place of the redox basis species Fe$^{3+}$.

Precipitation of hematite and goethite is disallowed.

In order to complete the description of surface complexation, the surface potential $\Psi$ must be specified.  This requires the specific surface area, which is assumed to be
\begin{equation}
A_{s} = 600\,\mathrm{m}^{2}/\mathrm{g(mineral)} \ ,
\end{equation}
We assume that:

- There is 1 free gram of Fe(OH)3(ppd), which sets $n_{k}$ for this mineral, and sets the surface area $A_{\mathrm{sf}}=600\,$m$^{2}$ in the formula for $\Psi$.
- For each mol of Fe(OH)3(ppd), there is 0.005$\,$mol of >(s)FeOH
- For each mol of Fe(OH)3(ppd), there is 0.2$\,$mol of >(w)FeOH

All of the above information is contained in the MOOSE database ????

The chemical composition of the water is 0.1 molal Na$^{+}$ and 0.1 molal Cl$^{-}$, and its temperature is 25$^{\circ}$C.

pH is initialised to 4 and progressively increased to 12.

[!cite](bethke_2007) presents results in Figures 14.8 and 14.9 (bold line only).

MOOSE produces the result ????

!bibtex bibliography