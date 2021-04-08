# Flushing minerals

Chapter 13.3 of [!cite](bethke_2007) describes a "flush" process, in which a specified solution is added to the system, and at the same time an equal volume of the equilibrium solution, consisting of solvent water, primary aqueous species, disolved minerals and disolved gases, is removed.  In the code, this is achieved by altering $M_{w}$, $M_{i}$, $M_{k}$ and $M_{m}$ appropriately.

Section 30.2 of [!cite](bethke_2007) describes an example of this involving alkali flooding of a petroleum reservoir.  It is assumed that quartz dissolves and precipitates with rate
\begin{equation}
r = A_{s}k \sqrt{a_{\mathrm{H}^{+}}} \left(1 - \frac{Q}{K}\right) \ ,
\end{equation}
where

- $A_{s}$ \[cm$^{2}$\] is set through a specific surface area of $A = 1000\,$cm$^{2}$/g(quartz);
- $k = 1.8\times 10^{-18}\,$mol.cm$^{-2}$.s$^{-1} = 1.5552\times 10^{-13}\,$mol.cm$^{-2}$.day$^{-1}$.
- $a_{\mathrm{H}^{+}}$ is the activity of H$^{+}$.

The initial configuration is given by:

- 1 molal Na$^{+}$;
- 0.2 molal Ca$^{2+}$;
- 1 molal Cl$^{-}$ (charge-balance species);
- $T=70^{\circ}$C;
- pH $=5$.

The initial minerals are:

- 9.88249$\,$mol ($\approx 365\,$cm$^{3}$) of free Calcite (in place of HCO$_{3}^{-}$ in the basis);
- 3.6525$\,$mol ($\approx 235\,$cm$^{3}$) of free Dolomite-ord (in place of Mg$^{2+}$);
- 1.27923$\,$mol ($\approx 180\,$cm$^{3}$) of free Muscovite (in place of K$^{+}$);
- 1.20579$\,$mol ($\approx 120\,$cm$^{3}$) of free Kaolinite (in place of Al$^{3+}$);
- 226.99243$\,$mol ($\approx 5150\,$cm$^{3}$) of free Quartz (in place of SiO$_{2}$(aq)).

The simulation contains only the following minerals: Analcime, Calcite, Dawsonite, Dolomite-ord, Gibbsite, Kaolinite, Muscovite, Paragonite and Phlogopite

Over the course of 20 days, the following is flushed through the system:

- 10$\,$kg of H$_{2}$O;
- 5$\,$moles of Na$^{+}$;
- 5$\,$moles of OH$^{-}$.

## MOOSE: stage 1

This is run in MOOSE using a 2-stage approach.  The first stage finds the free molality of SiO2(aq) in the equilibrium configuration before the system is flushed.  It is necessary to find this because since Quartz is a kinetic mineral it cannot be part of the basis in the second stage, so SiO2(aq) takes its place in the basis in that stage.  The first stage is a time-independent simulation:

!listing modules/geochemistry/test/tests/time_dependent_reactions/flushing_equilibrium_at70degC.i

## MOOSE: stage 2

This stage adds the NaOH solution and uses a kinetically-controlled quartz.  The [GeochemicalModelDefinition](GeochemicalModelDefinition.md) defines the basis species and equilibrium minerals.  It also specifies that `Quartz` is a kinetic mineral:

!listing modules/geochemistry/test/tests/time_dependent_reactions/flushing.i start=[definition] end=[]

The kinetic rate for Quartz is specified by a [GeochemistryKineticRate](GeochemistryKineticRate.md) UserObject

!listing modules/geochemistry/test/tests/time_dependent_reactions/flushing.i start=[rate_quartz] end=[definition]

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) specifies:

- the swaps;
- the initial pH (via the H$^{+}$ `activity`), the bulk composition of the aqueous species (note the [difference for Ca++ compared with GWB](theory/gwb_diff.md)) and the free mole number for the minerals (not Quartz because it is a kinetic species) and free molality of SiO2(aq) from stage 1;
- the temperature;
- the initial number of moles for Quartz
- that the kinetic rate should be updated during the Newton process that finds the equilibrium configuration at each time-step (which is unnecessary and computationally inefficient for this example, but is included for illustration)
- that the system is closed at $t=0$, which is the default, which means that no more SiO2(aq) or H$_{2}$O can be added/removed from the system to maintain the free mole numbers of these species;
- that the fixed pH is removed at $t=0$;
- that "flush" mode is active;
- the reactant rate: H$_{2}$O is added at rate 27.72$\,$mol/day, and NaOH is added at rate 0.25$\,$mol/day.

!listing modules/geochemistry/test/tests/time_dependent_reactions/flushing.i block=TimeDependentReactionSolver

The `Executioner` defines the time-stepping and provides physical meaning to "time"

!listing modules/geochemistry/test/tests/time_dependent_reactions/flushing.i block=Executioner

The graphs below were generated using `dt = 0.2` but the file in the test suite uses a larger time-step for efficiency.

A set of `Postprocessors` record the desired information using the `AuxVariables` automatically added by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md).  For example:

!listing modules/geochemistry/test/tests/time_dependent_reactions/flushing.i start=[pH] end=[cm3_Analcime]

## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/time_dependent_reactions/flushing.rea

Note the different initial molality specified for Ca$^{2+}$.  In this case it makes no noticable difference to the result, but users desiring an exact correspondance between the two software packages should understand [the differences](theory/gwb_diff.md).

## Results

Figures 30.3 and 30.4 in [!cite](bethke_2007) shows the results.  The GWB and `geochemistry` module results are shown in the figures below.

!media flushing_1.png caption=pH during the alkali flooding experiment.  Compare with Bethke's Figure 30.3  id=flushing_1_fig

!media flushing_2.png caption=Volume change of the precipitating minerals during the alkali flooding experiment.  Compare with Bethke's Figure 30.4  id=flushing_2_fig

!media flushing_3.png caption=Volume change of the dissolving minerals during the alkali flooding experiment.  Compare with Bethke's Figure 30.4  id=flushing_3_fig

!media flushing_4.png caption=Volume change of the kinetically-controlled quartz mineral during the alkali flooding experiment.  Compare with Bethke's Figure 30.4  id=flushing_4_fig





!bibtex bibliography