# Dumping minerals and then adding chemicals

Chapter 13.3 of [!cite](bethke_2007) describes a "dump" process, whereby a mineral is added to the initial solution, equilibrium sought, then any undissolved mineral is removed.  In the code, this is achieved by:

- subtracting $n_{k}$ from $M_{k}$;
- then setting $n_{k}=0$;
- setting the mole numbers of any surface components to zero, $M_{p}=0$, as well as the molalities of unoccupied surface sites, $m_{p}=0$ and adsorbed species, $m_{q}=0$;
- finally, swapping an appropriate aqueous species $A_{j}$ into the basis in place of $A_{k}$.

After this process, other chemicals can be added to this system, and/or the temperature varied, and/or the pH varied, etc.

Section 15.2 of [!cite](bethke_2007) provides an example of this "dump" process.  Assume:

- the mineral calcite is used in the basis instead of HCO$_{3}^{-}$, and that the free mole number of the calcite is 0.2708 (corresponding to a free volume of calcite is $\approx 10\,$cm$^{3}$);
- the pH is initially 8;
- the concentration of Na$^{+}$ is 100$\,$mmolal;
- the concentration of Ca$^{2+}$ is 10$\,$mmolal;
- charge balance is enforced on Cl$^{-}$.

## MOOSE input file

The [GeochemicalModelDefinition](GeochemicalModelDefinition.md) defines the basis species, the equilibrium mineral and gas of interest in this problem, as well as using the `piecewise_linear_interpolation` flag for accurate comparisons with the [Geochemists Workbench](https://www.gwb.com/) software:

!listing modules/geochemistry/test/tests/time_dependent_reactions/calcite_dumping.i block=UserObjects

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) defines:

- the swaps;
- the initial bulk mole number of the aqueous species [in the current basis](theory/gwb_diff.md), the free mole number of Calcite and the pH (via the H$^{+}$ `activity`);
- that the system is closed at $t=0$ (the default), so that after this time no more Calcite can be added to the system to maintain its free mole number;
- that at $t=0$ the activity constraint is removed on H$^{+}$ so that the pH can vary;
- that HCl is added at a rate of 0.001$\,$mol/s;
- using `mode = 1`, that Calcite is dumped from the system after each time-step (calcite never precipitates in this example, so the dumping actually only happens at the first time-step).

!listing modules/geochemistry/test/tests/time_dependent_reactions/calcite_dumping.i block=TimeDependentReactionSolver

The `Executioner` provides meaning to time, in particular that 100$\,$s is used, so a total of 0.1$\,$mol of HCl is added:

!listing modules/geochemistry/test/tests/time_dependent_reactions/calcite_dumping.i block=Executioner

The results presented below were run with a time-step of 1 to improve the appearance of the graphs.

A set of `Postprocessors` record the desired information using `AuxVariables` automatically added by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md):

!listing modules/geochemistry/test/tests/time_dependent_reactions/calcite_dumping.i block=Postprocessors

## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) file is

!listing modules/geochemistry/test/tests/time_dependent_reactions/calcite_dumping.rea

Note the slightly increased Ca$^{2+}$ bulk composition: this is explained [here](theory/gwb_diff.md).

## Results

Two cases are run:

- The system is brought to equilibrium and then the calcite is "dumped" using `mode = 1` in the input file.  After this, 100$\,$mmol of HCl is added to the system.
- The system is brought to equilibrium and but the calcite is not "dumped" using `mode = 0` in the input file.  After this, 100$\,$mmol of HCl is added to the system.

[!cite](bethke_2007) presents the results in Figures 15.6 and 15.7.  Both GWB and the `geochemistry` module produce the same results, as shown below.

!media calcite_dumping_1.png caption=CO2(g) fugacity as HCl is added to the fluid.  Compare with Bethke's Figure 15.6  id=calcite_dumping_1_fig

!media calcite_dumping_2.png caption=pH as HCl is added to the fluid.  Compare with Bethke's Figure 15.6  id=calcite_dumping_2_fig

!media calcite_dumping_3.png caption=Species concentrations as HCl is added to the fluid.  Compare with Bethke's Figure 15.7  id=calcite_dumping_3_fig


!bibtex bibliography