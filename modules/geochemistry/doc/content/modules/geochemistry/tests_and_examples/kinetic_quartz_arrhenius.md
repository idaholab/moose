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
- initially 400$\,$g (6.657313$\,$mol) of quartz is added to the water;
- the specific surface area is $A_{s} = 2.35\times 10^{-5}\,$cm$^{2}$/g(quartz);
- the activation energy is $E=72800\,$J.mol$^{-1}$;
- all other silica-containing minerals are prevented from precipitating.

## MOOSE input file: stage 1

The MOOSE simulation is in 2 stages.  The first determines the molality of SiO2(aq) that is in equilibrium with the quartz.  This is necessary because the problem description assumes that the water has had enough time to equilibrate with the quartz mineral at 300$^{\circ}$C, and in this stage the quartz mineral is not governed by a kinetic rate.

The system described in the input file also includes very small amounts of Na$^{+}$ and Cl$^{-}$.  These do not impact the results but are necessary because the `geochemistry` module requires a charge-balance species to be defined.

!listing modules/geochemistry/test/tests/kinetics/quartz_equilibrium_at300degC.i

The output of this simulation is that the molality of SiO2(aq) is 0.009723$\,$mol.

## MOOSE input file: stage 2

The second stage uses this molality and performs the time-dependent simulation, as the temperature is reduced.  The [GeochemistryKineticRate](GeochemistryKineticRate.md) is defined:

!listing modules/geochemistry/test/tests/kinetics/quartz_deposition.i start=[rate_quartz] end=[definition]

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) defines the free molality of SiO2(aq) at the initial time, the initial mole number of quartz and that the temperature is controlled using the `temp_controller` `AuxVariable`:

!listing modules/geochemistry/test/tests/kinetics/quartz_deposition.i block=TimeDependentReactionSolver

The temperature controller is:

!listing modules/geochemistry/test/tests/kinetics/quartz_deposition.i start=[temp_controller_auxk] end=[diss_rate]

with time defined through:

!listing modules/geochemistry/test/tests/kinetics/quartz_deposition.i block=Executioner

The figures below were generated using a time-step of 0.001$\,$yr.  A set of `AuxVariables` and `Postprocessors` define the desired output using the `mg_per_kg_SiO2(aq)` variable automatically included by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md):

!listing modules/geochemistry/test/tests/kinetics/quartz_deposition.i start=[AuxVariables]

## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) file is

!listing modules/geochemistry/test/tests/kinetics/quartz_deposition.rea

## Results

[!cite](bethke_2007) presents results in Figures 26.3 and 26.4, which look like:

!media quartz_deposition1.png caption=Change in free mass of SiO2(aq) as fluid flows through a fracture, changing temperature as it does so.  Compare with Bethke's Figure 26.3  id=quartz_deposition1.fig

!media quartz_deposition2.png caption=Quartz reaction rate as fluid flows through a fracture, changing temperature as it does so.  Compare with Bethke's Figure 26.4  id=quartz_deposition2.fig

The accuracy of the `geochemistry` simulation depends on the time-step size.  The above figures were generated using a step size of 0.001$\,$yr.

!bibtex bibliography
