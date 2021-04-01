# Flow-through reactions that remove minerals

Chapter 13.3 of [!cite](bethke_2007) describes a "flow-through" process, whereby a mineral is removed from the system at each stage in a reaction process (such as progressively adding chemicals, changing the temperature, changing the pH, etc).  In the code, this is achieved by the following process, after each stage's equilibrium configuration is computed:

- subtracting $n_{k}$ from $M_{k}$;
- then setting $n_{k}$ to a tiny number (but not zero, otherwise it might be swapped out of the basis);
- setting the mole numbers of any surface components to zero, $M_{p}=0$, as well as the molalities of unoccupied surface sites, $m_{p}=0$ and adsorbed species, $m_{q}=0$.

Section 24.3 of [!cite](bethke_2007) describes an example of this involving the evaporation of seawater.  Note that [!cite](bethke_2007) uses the HMW activity model, but `geochemistry` uses the Debye-Huckel approach, so the MOOSE results are not identical to [!cite](bethke_2007).  Nevertheless, it is hoped that the following description helps users set up similar models.

The initial composition of the seawater is shown in [table:analysis].  In addition:

- CO$_{2}$(g) is used in the basis instead of H$^{+}$;
- the fugacity of CO$_{2}$(g) is fixed to $10^{-3.5}$;
- after equilibration, all minerals are [dumped](calcite_buffer.md) from the system.

!table id=table:analysis caption=Major element composition of seawater
| Species | Moles | Approx conc (mg.kg$^{-1}$) |
| --- | --- | --- |
| Cl$^{-}$ | 0.5656 | 19350 |
| Na$^{+}$ | 0.4850 | 10760 |
| SO$_{4}^{2-}$ | 0.02924 | 2710 |
| Mg$^{2+}$ | 0.05501 | 1290 |
| Ca$^{2+}$ | 0.01063 | 411 |
| K$^{+}$ | 0.010576 | 399 |
| HCO$_{3}^{-}$ | 0.002412 | 142 |

Two cases are run.  Each involves gradually removing 55$\,$mol of H$_{2}$O from the solution.  The cases are:

1. As the water is removed, minerals are allowed to precipitate, and then potentially dissolve, to maintain equilibrium.
2. As the water is removed, minerals are allowed to precipitate, but when they precipitate they are immediately removed from the system following the "flow-through" method.

Only 6$\,$g of solvent water remains that the end of each simulation.


## MOOSE input file: flow through

The [GeochemicalModelDefinition](GeochemicalModelDefinition.md) defines the basis species, the equilibrium minerals and gases:

!listing modules/geochemistry/test/tests/time_dependent_reactions/seawater_evaporation_flow_through.i block=UserObjects

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) defines:

- the swap;
- the bulk composition of the aqueous species as well as the CO$_{2}$(g) fugacity via its `activity`;
- that the system is closed at $t=0$ (the default) which, in this case, means that no more solvent water may be added to the system to maintain the 1.0$\,$kg of solvent water;
- that the fugacity constraint remains throughout the simulation (there is are `remove_activity` lines);
- that H$_{2}$O is removed from the system at a rate of 1.0$\,$mol/s.
- the `mode` is defined via the AuxVariable called "mode"

!listing modules/geochemistry/test/tests/time_dependent_reactions/seawater_evaporation_flow_through.i block=TimeDependentReactionSolver

The `TimeStepper` and `Executioner` define the meaning of time.  In particular, the simulation is run for 55$\,$s, so that 55$\,$mol of water is removed::

!listing modules/geochemistry/test/tests/time_dependent_reactions/seawater_evaporation_flow_through.i start=[Functions] end=[Outputs]

Note that a smaller time-step size (`y = '1 0.1 0.001'`) was used to generate the figures below, but the test-suite file uses larger time-steps for efficiency.

The `mode` is defined by the `AuxKernel`

!listing modules/geochemistry/test/tests/time_dependent_reactions/seawater_evaporation_flow_through.i start=[mode_auxk] end=[dolomite_mol_auxk]


A set of `AuxVariables` and `AuxKernels` such as

!listing modules/geochemistry/test/tests/time_dependent_reactions/seawater_evaporation_flow_through.i start=[dolomite_mol_auxk] end=[gypsum_mol_auxk]

record the desired quantities.

## MOOSE input file: no flow through

The no flow-through case is similar (just varying in the `mode`, the `AuxKernels` and the `Postprocessors`):

!listing modules/geochemistry/test/tests/time_dependent_reactions/seawater_evaporation_no_flow_through.i


## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/time_dependent_reactions/seawater_evaporation.rea

## Results

Using the HMW activity model means that the results of case 1 are very different from case 2, as shown in Figures 24.7, 24.8 and 24.9 of [!cite](bethke_2007).  Using the Debye-Huckel model means the results of both cases are quite similar: the impact of the flow-through is evident towards the end of the simulation on the mirabilite mineral in particular.  The [Geochemists Workbench](https://www.gwb.com/) and the `geochemistry` module results are shown below.

!media seawater_evaporation_2.png caption=Minerals precipitated as seawater evaporates and flow-through is used.  id=seawater_evaporation_fig2

!media seawater_evaporation_1.png caption=Minerals precipitated and dissolved as seawater evaporates (no flow-through is used).  id=seawater_evaporation_fig1


!bibtex bibliography