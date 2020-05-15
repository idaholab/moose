# EquilibriumReactionSolver

This sets up a MOOSE simulation to solve an equilibrium reaction system.  There are zero time steps, no "usual" solution process, no "usual" output, etc.  The only thing the simulation does is add a [EquilibriumReactionSolverOutput](EquilibriumReactionSolverOutput) Output, which solves the reaction system, and outputs molalities, activities, etc (and then MOOSE exits).

An example is

!listing modules/geochemistry/test/tests/equilibrium_models/seawater_no_precip.i


