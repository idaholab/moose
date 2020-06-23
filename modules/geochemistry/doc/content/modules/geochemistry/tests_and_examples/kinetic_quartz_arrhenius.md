# Quartz deposition in a fracture

Section 26.2 of [!cite](bethke_2007) describes quartz deposition in a hydrothermal fracture.  The setup is similar to [quartz dissolution](kinetic_quartz.md) but with a different form of the reaction rate.  The reaction is
\begin{equation}
\mathrm{Quartz} \rightarrow \mathrm{SiO}_{2}\mathrm{(aq)} \ ,
\end{equation}
with rate
\begin{equation}
r = A_{s} e^{-E/(RT)}\left( 1 - \frac{Q}{K} \right) \ .
\end{equation}
It is assumed that:

- there is 1$\,$kg of water;
- the initial temperature is 300$^{\circ}$C and this steadily reduces to 25$^{\circ}$C over the course of 1 year;
- the mineral quartz is used instead of SiO$_{2}$(aq) in the basis initially while quartz is an equilibrium mineral (before it is kinetically-controlled);
- initially 400$\,$g of quartz is added to the water;
- the specific surface area is $A_{s} = 2.35\times 10^{-5}\,$cm$^{2}$/g(quartz);
- the activation energy is $E=72800\,$J.mol$^{-1}$;
- all other silica-containing minerals are prevented from precipitating.

[!cite](bethke_2007) presents results in Figures 26.3 and 26.4.

The MOOSE simulation is in 2 stages.  First first determines the molality of SiO2(aq) that is in equilibrium with the quartz:

!listing modules/geochemistry/test/tests/kinetics/quartz_equilibrium_at300degC.i

The second stage uses this molality and performs the time-dependent simulation, as the temperature is reduced:

!listing modules/geochemistry/test/tests/kinetics/quartz_deposition.i



!bibtex bibliography