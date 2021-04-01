# Adding fluids with different temperatures

Section 22.2 of [!cite](bethke_2007) describes the mixing of different fluids, which Bethke terms as "pickup".

## Step 1

The simulation begins with forming an equilibrium solution of seawater.  The major element composition is shown in [table:seawater].  In addition:

- $T=4^{\circ}$C;
- pH $=8.1$;
- the following minerals are prevented from precipitating: Quartz, Tridymite, Cristobalite, Chalcedony and Hematite.

The system is equilibrated, and the final composition without precipitated minerals is saved

!table id=table:seawater caption=Major element composition of seawater
| Species | Concentration (mmolal) |
| --- | --- |
| Cl$^{-}$ | 559 |
| Na$^{+}$ | 480 |
| Mg$^{2+}$ | 54.5 |
| SO$_{4}^{2-}$ | 29.5 |
| Ca$^{2+}$ | 10.5 |
| K$^{+}$ | 10.1 |
| HCO$_{3}^{-}$ | 2.4 |
| SiO$_{2}$(aq) | 0.17 |
| Sr$^{2+}$ | 0.09 |
| Ba$^{2+}$ | $0.20\times 10^{-3}$ |
| Zn$^{2+}$ | $0.01\times 10^{-3}$ |
| Al$^{3+}$ | $0.005\times 10^{-3}$ |
| Cu$^{+}$ | $0.007\times 10^{-3}$ |
| Fe$^{2+}$ | $0.001\times 10^{-3}$ |
| Mn$^{2+}$ | $0.001\times 10^{-3}$ |
| O$_{2}$(aq) | 0.123 (free) |

## Step 2

The hot hydrothermal fluid has composition shown in [table:hydrothermal].  In addition:

- the temperature is 273$^{\circ}$C;
- the pH is 4;
- H$_{s}$S(aq) is used in the basis instead of O$_{2}$(aq).

The system is brought to equilibrium, then the minerals are allowed to precipitate, and then all precipitated minerals are [dumped](calcite_buffer.md).

!table id=table:hydrothermal caption=Major element composition of hydrothermal fluid
| Species | Concentration (mmolal) |
| --- | --- |
| Cl$^{-}$ | 600 |
| Na$^{+}$ | 529 |
| Mg$^{2+}$ | $0.01\times 10^{-3}$ |
| SO$_{4}^{2-}$ | $0.01\times 10^{-3}$ |
| Ca$^{2+}$ | 21.6 |
| K$^{+}$ | 26.7 |
| HCO$_{3}^{-}$ | 2.0 |
| Ba$^{2+}$ | $15\times 10^{-3}$ |
| SiO$_{2}$(aq) | 20.2 |
| Sr$^{2+}$ | $100.5\times 10^{-3}$ |
| Zn$^{2+}$ | $41\times 10^{-3}$ |
| Cu$^{+}$ | $0.02\times 10^{-3}$ |
| Al$^{3+}$ | $4.1\times 10^{-3}$ |
| Fe$^{2+}$ | $903\times 10^{-3}$ |
| Mn$^{2+}$ | $1039\times 10^{-3}$ |
| H$_{2}$S(aq) | 6.81 |

## Step 3

The seawater at 4$^{\circ}$C is slowly mixed into the geothermal fluid at 273$^{\circ}$C until the final ratio is 10(seawater):1(geothermal).  A constant heat capacity is assumed.

## MOOSE input file

MOOSE input files dealing with the equilibration (`seawater_mixing_step1.i` and `seawater_mixing_step2.i`) may be found in the test suite.  The focus here is on the mixing.  The [GeochemicalModelDefinition](GeochemicalModelDefinition.md) defines the basis species, the relevant minerals and the gas (so that its fugacity is computed by the simulation):

!listing modules/geochemistry/test/tests/time_dependent_reactions/mixing.i block=UserObjects

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) needs some discussion:

!listing modules/geochemistry/test/tests/time_dependent_reactions/mixing.i block=TimeDependentReactionSolver

From the top to the bottom of this block:

- the swaps are defined
- the composition of the fluid is defined via the bulk mole composition of the species as well as the activity of H$^{+}$ which sets the pH
- the system is closed at $t=-0.01$.  This means that all "free" type constraints are converted into "bulk" type constraints.  The only such constraint in this case is on H$_{2}$O, so after $t=-0.1$ no more H$_{2}$O can be added to the system to maintain the `kg_solvent_water` constraint.
- the activity constraint on H$^{+}$ is also removed at $t=-0.1$.  This means that no more H$^{+}$ can be added to the system to maintain the pH constraint
- the initial temperature is 273$^{\circ}$C, which is the temperature at which the system is initially equilibrated at
- the subsequent temperature is controlled by the `T` variable (see below)
- various source species are provided with rates (see below)
- the `mode` is set equal to the mode variable (see below)

The `Executioner` defines the meaning of time:

!listing modules/geochemistry/test/tests/time_dependent_reactions/mixing.i block=Executioner

The temperature is controlled using:

!listing modules/geochemistry/test/tests/time_dependent_reactions/mixing.i start=[T_auxk] end=[H2O_rate]

This means that during initialization the temperature is 273$^{\circ}$C, and whenever the source species' rates are nonzero (which is for $t>0$ in this case) then the reactant temperature is 4$^{\circ}$C.

The `mode` is set to "dump" for $t\leq 0$, and otherwise no special mode, by:

!listing modules/geochemistry/test/tests/time_dependent_reactions/mixing.i start=[mode_auxk] end=[T_auxk]

All the source-species rates follow the same pattern:

!listing modules/geochemistry/test/tests/time_dependent_reactions/mixing.i start=[H2O_rate_auxk] end=[Al+++_rate]

The numerical values of the rates were derived from the [Geochemists Workbench](https://www.gwb.com/) input file (below) so that the results match as closely as possible to the GWB results, but they could equally be derived from `seawater_mixing_step1.i` and `seawater_mixing_step2.i`.

## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/time_dependent_reactions/mixing.rea

## Results

Figures 22.3 and 22.4 of [!cite](bethke_2007) show the results, which may be compared with the results below.  Note, there are two reasons why the two software packages produce slightly different results.

1. Most importantly, the interpolations of equilibrium constants at high temperature appears to be different in the two software packages.  This may be verified by running the other simulations in the [tests and examples](geochemistry/tests_and_examples/index.md) at higher temperatures and observing that the results begin to differ at higher temperatures, despite being identical at lower temperatures.  In the current situation, this mostly impacts the system at $T\approx 150^{\circ}$C and causes aspects of the models to differ.

2. Less importantly, during the initial equilibration process, [Geochemists Workbench](https://www.gwb.com/) first equilibrates the solution, then removes the pH constraint, then allows precipitates to form, then dumps the precipitates.  Conversely, the `geochemistry` module first equilibrates the solution allowing precipitates to form, then dumps the precipitates and removes the pH constraint.  The difference between these two approaches is that during the precipitation `geochemistry` is adding/removing HCl to the aqueous solution to maintain the user-specified pH constraint.  This means the initial configuration before adding the seawater is slightly different, and explains the different behaviour of the Talc mineral during the early stages of the simulation (which may be verified by creating another model with fixed bulk mole number for H$^{+}$).

!media mixing_1.png caption=Aqueous solution temperature.  id=mixing_fig1

!media mixing_2.png caption=Precipitates formed.  id=mixing_fig2

!media mixing_3.png caption=Oxygen fugacity.  id=mixing_fig3

!media mixing_4.png caption=Species concentration.  id=mixing_fig4



!bibtex bibliography