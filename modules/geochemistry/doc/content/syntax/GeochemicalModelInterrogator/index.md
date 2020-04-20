# GeochemicalModelInterrogator

This sets up a very simple MOOSE simulation, with zero time steps, no "usual" solution process, no "usual" output, etc.  The only thing the simulation does is add a [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) userobject, which outputs [balanced reactions and equilibrium constants](reaction_balancing.md), [activity ratios](activity_ratios.md), [equilibrium temperature](eqm_temp_a.md), etc (and then MOOSE exits).  See the [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md) userobject for more discussion.

An example is

!listing modules/geochemistry/test/tests/interrogate_reactions/clinoptilolite.i


