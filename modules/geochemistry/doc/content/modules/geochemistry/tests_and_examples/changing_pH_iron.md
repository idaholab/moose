# Exploring the impact of pH on sorption

Section 14.3 of [!cite](bethke_2007) describes how to explore the impact of pH on sorption.

This page builds upon the [surface complexation example](surface_complexation.md) in which sorption to the ferric hydroxide mineral Fe(OH)$_{3}$ was studied.  In that example, agreement between [!cite](bethke_2007), the [Geochemists Workbench](https://www.gwb.com/) software, and the `geochemistry` module was demonstrated at pH 4 and 8.  On this page, the pH is varied between 4 and 12, and amount of sorption of each species is plotted.

The [surface complexation example](surface_complexation.md) included lead, mercury and iron complexes, but for simplicity this example only include iron complexes.  The model definition therefore reads:

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_pH_ferric_hydroxide.i block=UserObjects

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) defines the initial composition as well as how to vary the pH with time via the `controlled_activity` inputs:

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_pH_ferric_hydroxide.i block=TimeDependentReactionSolver

The `set_aH` value is defined via a `FunctionAux` in the following way:

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_pH_ferric_hydroxide.i start=[AuxVariables] end=[Postprocessors]

and the timestepping is defined in the executioner:

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_pH_ferric_hydroxide.i block=Executioner

In this situation, the `start_time` is before $t=0$ when the system closes and the time-dependency begins, just so the results at $\mathrm{pH}=4$ can be simply recorded.  Finally, the quantities of interest are recorded into `Postprocessors` using the `AuxVariables` that are automatically included in the simulation by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md)

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_pH_ferric_hydroxide.i block=Postprocessors

[!cite](bethke_2007) presents results in Figures 14.8 and 14.9 (bold line only, since the fine line shows a different type of aqueous solution).  The results are faithfully reproduced by the `geochemistry` module as shown in the figures below.

!media changing_pH_ferric_hydroxide_fig1.png caption=Concentrations of sites on a ferric oxide surface.  Compare with Bethke's Figure 14.8  id=changing_pH_ferric_hydroxide_fig1

!media changing_pH_ferric_hydroxide_fig2.png caption=Surface potential of ferric oxide.  Compare with Bethke's Figure 14.9  id=changing_pH_ferric_hydroxide_fig2



!bibtex bibliography