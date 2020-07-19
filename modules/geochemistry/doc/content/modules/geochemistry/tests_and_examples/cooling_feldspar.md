# Cooling a solution in contact with feldspars

This page follows section 14.1 of [!cite](bethke_2007) and involves computing the equilibrium system as a solution in contact with feldspars is cooled.

The initial composition of the water is shown in [table:composition].  In addition, the "albite low" mineral is supressed (its precipitation is disallowed).  The system is initialised at 300$^{\circ}$C and slowly cooled to 25$^{\circ}$C.

!table id=table:composition caption=Initial composition
| Species | Concentration |
| --- | --- |
| Na$^{+}$ | 1 molal (bulk) |
| Cl$^{-}$ | 1 molal (bulk) |
| Albite (in place of Al$^{3+}$) | 20 cm$^{3}$ (free) |
| Maximum microcline (in place of K$^{+}$) | 10 cm$^{3}$ (free) |
| Muscovite (in place of H$^{+}$) | 5 cm$^{3}$ (free) |
| Quartz (in place of SiO$_{2}$(aq) | 2 cm$^{3}$ (free) |

[!cite](bethke_2007) predicts that microcline slowly precipitates, albite slowly dissolves, while muscovite and quartz remain largely unchanged (see [!cite](bethke_2007) Figure 14.1)

## MOOSE input file: model definition

The MOOSE input file defines the basis species and equilibrium minerals via the [GeochemicalModelDefinition](GeochemicalModelDefinition.md).  Note the appearance of the `remove_all_extrapolated_secondary_species` flag, which is necessary to ensure convergence of this problem.  The offending secondary species is `Al13O4(OH)24(7+)` that has $\log_{10}K$ defined only at low temperatures in the original database.  The default process to use for such species is to extrapolate the equilibrium constants to higher temperatures.  In this case $K = 10^{-192.5684}$ at $T=300^{\circ}$C which prevents `geochemistry` from converging, so the species is removed (it is also removed by the [Geochemists Workbench](https://www.gwb.com/) software).

!listing modules/geochemistry/test/tests/time_dependent_reactions/cooling.i block=UserObjects

## MOOSE input file: model time-dependence

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) defines:

- the swaps mentioned above
- the charge-balance species
- the constraints on the species, which in this case hold just at the initial time because the system is closed after that time (no more minerals are added after the initialization, but they could be added during the initialization to ensure the correct number of free moles)
- the initial temperature
- the temperature as a function of time
- other flags that allow accurate comparison with the [Geochemists Workbench](https://www.gwb.com/) software

!listing modules/geochemistry/test/tests/time_dependent_reactions/cooling.i block=TimeDependentReactionSolver

!alert note
The total bulk mole number of Na+ and Cl- is set at 1.14093 instead of 1.0 as set above.  This is to compare with the GWB result (see below).

The time-dependent temperature is defined through an `AuxVariable`:

!listing modules/geochemistry/test/tests/time_dependent_reactions/cooling.i start=[AuxVariables] end=[Postprocessors]

along with:

!listing modules/geochemistry/test/tests/time_dependent_reactions/cooling.i block=Executioner

## MOOSE input file: recording the desired quantities

The desired mineral volumes and solution temperature are recorded into `Postprocessors` using the `AuxVariables` automatically included by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md):

!listing modules/geochemistry/test/tests/time_dependent_reactions/cooling.i block=Postprocessors

## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/time_dependent_reactions/cooling.rea

!alert warning
Although the bulk composition for Na+ and Cl- are specified as 1 molal, GWB changes the mole number of Cl- to 1.14 to ensure charge balance.  That is the reason why the bulk composition is set at 1.14 in the MOOSE input file.  In this case the results are virtually unchanged.

## Results

!media cooling.png caption=Precipitated volumes as a function of temperature  id=cooling_ppd.fig






!bibtex bibliography