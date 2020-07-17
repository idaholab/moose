# Solubility of gypsum and associated activities

This example closely follows Section 8.3 of [!cite](bethke_2007).

The solubility of gypsum (CaSO$_{4}$.2H$_{2}$O) as a function of NaCl concentration is explored.  A simulation is initialized with some undissolved Gypsum and small quantities of Na$^{+}$ and Cl$^{-}$, and then NaCl is progressively added.  The following assumptions are made:

- Gypsum is used in the basis instead of the aqueous species Ca$^{2+}$.
- The initial amount of free gypsum is 0.5814$\,$mol ($\approx 100\,$g).
- Charge balance is performed on SO$_{4}^{2-}$.

Hence, the basis is (H20, Na+, Cl-, SO4--, gypsum).  The simulation computes how much gypsum is dissolved as a function of the concentration of NaCl.

The results are shown in Figure 8.6 of [!cite](bethke_2007).

## The MOOSE input file

The input file begins by defining the basis aqueous species and the equilibrium minerals (only Gypsum) for the model.  The `piecewise_linear_interpolation` flag is used for accurate comparison with the [Geochemists Workbench](https://www.gwb.com/) software.

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.i block=UserObjects

A [TimeDependentReactionSolver](actions/AddTimeDependentReactionSolverAction.md) is used to simulate the adding of NaCl:

- Ca$^{2+}$ is swapped out of the basis in favor of Gypsum.
- Na$^{+}$ and Cl$^{+}$ are provided with a small initial molality.  The molality for SO$_{4}^{2-}$ will actually be controlled by charge neutrality.
- 0.5814 free moles of Gypsum are introduced into the initial system
- During the initial setup, the `geochemistry` module will find the equilibrium configuration corresponding to these initial conditions.  By default, it will then close the system (the `close_system_at_time` input defaults to 0.0).  This stops any further Gypsum from entering the system.
- The rate of addition of NaCl is defined to be 1.0$\,$mol/s
- The other flags enable an accurate comparison with the [Geochemists Workbench](https://www.gwb.com/) software.

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.i block=TimeDependentReactionSolver

The time-stepping and output are defined in the usual MOOSE way.  In this case, a `FunctionDT` timestepper is used to capture the nonlinear behaviour at the beginning of the sinulation:

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.i block=Executioner

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.i block=Outputs

An `AuxVariable` captures the amount of dissolved gypsum by using the `bulk_moles_Gypsum` and `free_mg_Gypsum` `AuxVariables` that are automatically added by the [TimeDependentReactionSolver](actions/AddTimeDependentReactionSolverAction.md) is used to simulate the adding of NaCl:

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.i block=AuxVariables

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.i block=AuxKernels

Finally, `Postprocessors` allow the desired information to be written to the CSV output file

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.i block=Postprocessors

## GWB input file

An equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/solubilities_and_activities/gypsum_solubility.rea

A total of 3 moles of NaCl is added to the system, which is equivalent to the 1$\,$mol/s over 3$\,$s used by the MOOSE input file.  To extract the results from [Geochemists Workbench](https://www.gwb.com/) its inbuilt graphing capability may be utilized, or the Cl$^{-1}$ molality vs the moles of Ca$^{2+}$ in the solution may be extracted from plaintext output file.

## Results

The results are shown in [gypsum_solubility.fig].

!media gypsum_solubility.png caption=Gypsum solubility as a function of chlorine molality.  id=gypsum_solubility.fig



!bibtex bibliography