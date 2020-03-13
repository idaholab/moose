# Flushing minerals

Chapter 13.3 of [!cite](bethke_2007) describes a "flush" process, in which a specified solution is added to the system, and at the same time an equal volume of the equilibrium solution, consisting of solvent water, primary aqueous species, disolved minerals and disolved gases, is removed.  In the code, this is achieved by altering $M_{w}$, $M_{i}$, $M_{k}$ and $M_{m}$ appropriately.

Section 30.2 of [!cite](bethke_2007) describes an example of this involving alkali flooding of a petroleum reservoir.  It is assumed that quartz dissolves and precipitates with rate
\begin{equation}
r = A_{s}k \sqrt{a_{\mathrm{H}^{+}}} \left(1 - \frac{Q}{K}\right) \ ,
\end{equation}
where

- $A_{s}$ \[cm$^{2}$\] is set through a specific surface area of $A = 1000\,$cm$^{2}$/g(quartz);
- $k = 1.6\times 10^{-18}\,$mol.cm$^{-2}$.s$^{-1}$;
- $a_{\mathrm{H}^{+}}$ is the activity of H$^{+}$.

The initial configuration is given by:

- 1 molal Na$^{+}$;
- 0.2 molal Ca$^{2+}$;
- 1 molal H$^{+}$;
- $T=70^{\circ}$C;
- pH $=5$.

The initial minerals are:

- $365\,$cm$^{3}$ of free Calcite (in place of HCO$_{3}^{-}$ in the basis);
- $235\,$cm$^{3}$ of free Dolomite-ord (in place of Mg$^{2+}$);
- $180\,$cm$^{3}$ of free Muscovite (in place of K$^{+}$);
- $120\,$cm$^{3}$ of free Kaolinite (in place of Al$^{3+}$);
- $5150\,$cm$^{3}$ of free Quartz (in place of SiO$_{2}$(aq)).

In addition the following minerals are prevented from precipitating: Albite, Albite-high, Albite-low, Maximum Microcline, K-feldspar, Sanidine-high.

### Case 1

Ten times, over the course of 20 days, the following is flushed through the system:

- 0.1$\,$kg of H$_{2}$O;
- 0.05$\,$moles of Na$^{+}$;
- 0.05$\,$moles of OH$^{-}$.

### Case 2

Ten times, over the course of 20 days, the following is flushed through the system:

- 0.1$\,$kg of H$_{2}$O;
- 0.05$\,$moles of Na$^{+}$;
- 0.025$\,$moles of CO$_{3}^{2-}$.

I believe [!cite](bethke_2007) performs this by removing all OH$^{-}$ from the final result of Case 1 and replacing it with 0.25$\,$moles of CO$_{3}^{2-}$.

### Case 3

Ten times, over the course of 20 days, the following is flushed through the system:

- 0.1$\,$kg of H$_{2}$O;
- 0.025$\,$moles of Na$_{2}$O;
- 0.025$\,$moles of SiO$_{2}$(aq).

I believe [!cite](bethke_2007) performs this by removing all CO$_{3}^{-}$ and Na${+}$ from the final result of Case 2 and replacing it with 0.025$\,$moles of Na$_{2}$O and 0.025$\,$moles of SiO$_{2}$(aq).

### Results

Figures 30.3 and 30.4 in [!cite](bethke_2007) shows the results.

MOOSE produces the result ????







!bibtex bibliography