# Dissolution of pyrite, with and without fixed oxygen fugacity

Section 14.2 of [!cite](bethke_2007) and involves studying the dissolution of pyrite, with and without fixed oxygen fugacity.

The initial composition of the water is shown in [table:composition].  In addition, in the initial configuration:

- hematite is used in place of Fe$^{2+}$ and its free concentration is $6.26\times 10^{-6}\,$mol ($\approx 1\,$mg)
- O$_{2}$(g) is used in place of O$_{2}$(aq), and its fugacity is set to $0.2$;
- the pH is 6.5.

!table id=table:composition caption=Initial bulk composition
| Species | Concentration (molal) | Approx conc (mg.kg$^{-1}$) |
| --- | --- | --- |
| Ca$^{2+}$ | $9.98\times 10^{-5}$ | 4 |
| Mg$^{2+}$ | $4.114\times 10^{-5}$ | 1 |
| Na$^{+}$ | $8.7\times 10^{-5}$ | 2 |
| HCO$_{3}^{-}$ | $2.95\times 10^{-4}$ | 18 |
| SO$_{4}^{2-}$ | $3.123\times 10^{-5}$ | 3 |
| Cl$^{-}$ (charge-balance) | $1.41\times 10^{-3}$ | 5 |
| Hematite (in place of Fe$^{2+}$) | $6.26\times 10^{-6}$ mol (free) | 1 mg (free) |

Two cases are studied:

- a case in which oxygen cannot or leave the system after initial equilibration
- a case in which the oxygen fugacity is fixed at 0.2 throughout the simulation

The MOOSE input files for both these cases are very similar.  A [GeochemicalModelDefinition](GeochemicalModelDefinition.md) defines the basis species, the equilibrium minerals and equilibrium gases:

!listing modules/geochemistry/test/tests/time_dependent_reactions/dissolution_pyrite_1.i block=UserObjects

An `Executioner` defines the time-stepping and end-time:

!listing modules/geochemistry/test/tests/time_dependent_reactions/dissolution_pyrite_1.i block=Executioner

The figures below were generated using a time-step size of 0.05, while the regression-test files use a larger time-step size for efficiency.

A set of `Postprocessors` define the desired outputs using the `AuxVariables` automatically added by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md):

!listing modules/geochemistry/test/tests/time_dependent_reactions/dissolution_pyrite_1.i block=Postprocessors


## Case 1: when oxygen cannot enter or leave the system

10$\,$mg of pyrite is slowly added to the system, without allowing any oxygen to enter or exit the system.

### MOOSE input file

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) involves specifying:

- the swaps
- the initial free mole number of Hematite, the pH via the H$^{+}$ activity, the bulk mole composition of the aqueous species, and the O2(g) fugacity
- the system is closed at $t=0$ (the default)
- that at $t=0$ the activity constraints are removed from H$^{+}$ (so the pH can vary during the simulation because an external agent is no longer fixing it) and O2(g) (meaning the external agent controlling the O2(g) fugacity is no longer adding/removing O2(g) from the aqueous solution)
- that 1$\,$mg/second of pyrite is added to the system

!listing modules/geochemistry/test/tests/time_dependent_reactions/dissolution_pyrite_1.i block=TimeDependentReactionSolver

### GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/time_dependent_reactions/dissolution_pyrite_1.rea

### Results

The following graphs demonstrate agreement between the `geochemistry` module and the GWB software, which may also be compared with [!cite](bethke_2007) Figure 14.2 and 14.3.

!media dissolution_pyrite_1_1.png caption=Precipitated volumes as pyrite is reacted in a system that is closed to oxygen  id=dissolution_pyrite_1_1.fig

!media dissolution_pyrite_1_2.png caption=Solution pH as pyrite is reacted in a system that is closed to oxygen  id=dissolution_pyrite_1_2.fig

!media dissolution_pyrite_1_3.png caption=Species concentrations as pyrite is reacted in a system that is closed to oxygen  id=dissolution_pyrite_1_3.fig


## Case 2: fixed oxygen fugacity

1000$\,$mg of pyrite is slowly added to the system, with fixed fugacity of oxygen $f_{\mathrm{O}_{2}}=0.2$ (so that oxygen can enter or leave the system)

### MOOSE input file

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) is almost identical to Case 1.  The differences are:

- that at $t=0$ the activity constraints are removed from H$^{+}$, but not O2(g)
- that 1000$\,$mg/second of pyrite is added to the system

!listing modules/geochemistry/test/tests/time_dependent_reactions/dissolution_pyrite_2.i block=TimeDependentReactionSolver

### Results

The following graphs demonstrate agreement between the `geochemistry` module and the GWB software, which may also be compared with [!cite](bethke_2007) Figure 14.4.

!media dissolution_pyrite_2_1.png caption=Precipitated volumes as pyrite is reacted in a system that is open to oxygen  id=dissolution_pyrite_2_1.fig

!media dissolution_pyrite_2_2.png caption=Solution pH as pyrite is reacted in a system that is open to oxygen  id=dissolution_pyrite_2_2.fig

The two cases are clearly dramatically different.  In Case 1, about 8$\,$mg of pyrite dissolves, precipitating hematite, before hematite suddenly dissolves and no further pyrite dissolution occurs.  In Case 2, pyrite dissolution and hematite precipitation continues indefinitely in a rather acidic solution.


!bibtex bibliography