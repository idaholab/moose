# SpatialReactionSolver

This sets up a MOOSE simulation that is spatially and temporally dependent, with no "usual" solution process (with Kernels, etc).  This Action adds a [GeochemistrySpatialReactor](GeochemistrySpatialReactor.md) userobject, many AuxVariables corresponding to molality, free-mg, free-cm3, pH, etc using the [GeochemistryQuantityAux](GeochemistryQuantityAux.md) and a console output object, the [GeochemistryConsoleOutput](GeochemistryConsoleOutput.md) along with its [NearestNodeNumberUO](NearestNodeNumberUO.md) UserObject.

The input file must also contain the [GeochemicalModelDefinition](GeochemicalModelDefinition.md) (as all `geochemistry` simulations must), along with a definition of the Mesh and Executioner.

An simple example input file is:

!listing modules/geochemistry/test/tests/spatial_reactor/spatial_1.i

