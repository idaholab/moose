# Progressively adding potassium feldspar

This page follows Chapter 13 of [!cite](bethke_2007) and involves computing the equilibrium system as K-feldspar is added to a hypothetical water.

The initial composition of the water is shown in [table:composition].  Assume that the temperature is 25$^{\circ}$C and the initial pH is 5.  $0.15\,$cm$^{3}$ of K-feldspar is slowly added to this system.

!table id=table:composition caption=Initial bulk composition of the hypothetical water
| Species | Concentration (molal) | Approx conc (mg/kg(solvent water)) |
| --- | --- | --- |
| HCO$_{3}^{-}$ | $8.194\times 10^{-4}$ | 50 |
| Cl$^{-}$ | $8.463\times 10^{-4}$ | 30 |
| Ca$^{2+}$ | $3.743\times 10^{-4}$ | 15 |
| SO$_{4}^{2-}$ | $8.328\times 10^{-5}$ | 8 |
| Na$^{+}$ | $2.175\times 10^{-4}$ | 5 |
| Mg$^{+}$ | $1.234\times 10^{-4}$ | 3 |
| SiO$_{2}$(aq) | $4.993\times 10^{-5}$ | 3 |
| K$^{+}$ | $2.558\times 10^{-5}$ | 1 |
| Al$^{3+}$ | $3.706\times 10^{-8}$ | $10^{-3}$ |

## MOOSE input file: definition

The [GeochemicalModelDefinition](GeochemicalModelDefinition.md) specifies the database file to use, and in this case the basis species and equilibrium minerals.  The flag `piecewise_linear_interpolation = true` in order to compare with the Geochemists Workbench result

!listing modules/geochemistry/test/tests/time_dependent_reactions/add_feldspar.i block=UserObjects

## MOOSE input file: initial conditions and reaction rates

The [TimeDependentReactionSolver](actions/AddTimeDependentReactionSolverAction.md) defines the initial conditions and the desired reaction processes:

- The pH is defined through specifying the `activity` for H$^{+}$.  Note that at time zero, just after the initial equilibration, this activity constraint is removed via the `remove_fixed_activity` inputs.
- The bulk composition of the other chemical species are specified by the `moles_bulk_species`, as per [table:composition].
- The rate of addition of `K-feldspar` is defined to be $1.37779\times 10^{-3}\,$mol/s, which corresponds to $0.15\,$cm$^{3}$/s.
- The other flags are for efficiency and enable an accurate comparison with the [Geochemists Workbench](https://www.gwb.com/) software.

!listing modules/geochemistry/test/tests/time_dependent_reactions/add_feldspar.i block=TimeDependentReactionSolver

## MOOSE input file: time stepping

A total of 100 steps over the course of 1$\,$s is used in this simulation

!listing modules/geochemistry/test/tests/time_dependent_reactions/add_feldspar.i block=Executioner

## MOOSE input file: outputs

A set of Postprocessors captures the output, using the `AuxVariables` added automatically by the [TimeDependentReactionSolver](actions/AddTimeDependentReactionSolverAction.md):

!listing modules/geochemistry/test/tests/time_dependent_reactions/add_feldspar.i block=Postprocessors

## Results

[!cite](bethke_2007) predicts that the minerals kaolinite, muscovite, quartz, K-feldspar and phengite all precipitate, and that kaolinite dissolves, as the K-feldspar is gradually added to the solution and the results are presented in [!cite](bethke_2007) Figure 13.1.  These results are faithfully reproduced by the `geochemistry` module, as in [add_feldspar.fig].

!media add_feldspar.png caption=Minerals precipitated as K-feldspar is added to the system.  id=add_feldspar.fig


!bibtex bibliography