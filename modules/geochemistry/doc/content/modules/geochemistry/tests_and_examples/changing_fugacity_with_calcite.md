# The impact of CO$_{2}$ fugacity on the solubility of calcite

Section 14.3 of [!cite](bethke_2007) and involves studying the solubility of calcite in the presence of a progressively changing CO$_{2}$ gas buffer.

Assume that:

- the bulk composition of Na$^{+}$ is 10$\,$mmolal;
- the bulk composition of Cl$^{-1}$ is 10$\,$mmolal;
- the mineral calcite is used in place of Ca$^{2+}$ in the basis, and that initially it has a free mole number of 0.01354$\,$mol, corresponding to a volume of $\approx 0.5\,$cm$^{3}$;
- CO$_{2}$(g) is used in place of H$^{+}$ in the basis, and that its fugacity is initially $10^{-3.5}$;
- charge balance is maintained on HCO$_{3}^{-}$.

Then the fugacity of CO$_{2}$(g) is changed from $10^{-3.5}$ to 1, and the dissolution of calcite, the pH, and species concentrations are all recorded as a function of time.

## MOOSE input file

The MOOSE input file includes the [GeochemicalModelDefinition](GeochemicalModelDefinition.md) which defines the basis species, the equilibrium minerals and equilibrium gases in this case.  The `piecewise_linear_interpolation` flag is set true so that the equilibrium constants and Debye-Huckel parameters are evaluated at the 25$^{\circ}$C value provided in the database file without any least-squares best-fit smoothing.

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_fugacity_calcite.i block=UserObjects

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) specifies

- the swaps used
- the initial free molality of Calcite, the initial fugacity of CO$_{2}$(g) and the bulk mole number for the other species
- the system is closed at $t=0$ by default, meaning that the external agent cannot add or remove Calcite in order to maintain the free mole number of 0.01354 (ie, Calcite can precipitate or dissolve freely)
- that the activity (fugacity) of CO$_{2}$(g) is controlled by the `fug_co2` `AuxVariable`

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_fugacity_calcite.i block=TimeDependentReactionSolver

The `fug_co2` fugacity-controller is

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_fugacity_calcite.i block=AuxKernels

with `Executioner` defining the notion of time:

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_fugacity_calcite.i block=Executioner

The figures below were produced using a time-step size of 0.01 but the regression test uses a bigger `dt` for efficiency.

A number of `Postprocessors` capture information from the `AuxVariables` automatically included by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md):

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_fugacity_calcite.i block=Postprocessors


## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/time_dependent_reactions/changing_fugacity_calcite.rea

## Results

[!cite](bethke_2007) presents results in Figures 14.6 and 14.7.  The [Geochemists Workbench](https://www.gwb.com/) and `geochemistry` module results are shown below.

!media changing_fugacity_calcite_1.png caption=Calcite volume as CO$_{2}$ fugacity is varied.  Compare with Bethke's Figure 14.6  id=changing_fugacity_calcite_1_fig

!media changing_fugacity_calcite_2.png caption=pH of a solution containing calcite as CO$_{2}$ fugacity is varied.  Compare with Bethke's Figure 14.6  id=changing_fugacity_calcite_2_fig

!media changing_fugacity_calcite_3.png caption=Species concentrations as CO$_{2}$ fugacity is varied in a solution containing calcite.  Compare with Bethke's Figure 14.7  id=changing_fugacity_calcite_3_fig


!bibtex bibliography