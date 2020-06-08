# TimeDependentReactionSolver

This sets up a simple time-dependent MOOSE simulation, with no spatial dependence and no "usual" solution process (with Kernels, etc).  Apart from setting up a dummy Mesh, Variables, etc, this Action adds a [GeochemistryTimeDependentReactor](GeochemistryTimeDependentReactor.md) userobject, many AuxVariables corresponding to molality, free-mg, free-cm3, pH, etc using the [GeochemistryQuantityAux](GeochemistryQuantityAux.md) and a console output object, the [GeochemistryConsoleOutput](GeochemistryConsoleOutput.md).

Here are some simple of examples of where this Action is used in the test suite:

- [Progressively adding chemicals](adding_feldspar.md)
- [Progressively changing the temperature](cooling_feldspar.md)
- [Progressively adding chemicals with fixed fugacity](adding_pyrite.md)
- [Progressively changing fugacity](changing_fugacity_with_calcite.md)
- [Progressively changing pH](changing_pH_iron.md)
- [Adding fluids of different temperatures](pickup.md)
- [Dumping minerals then adding chemicals](calcite_buffer.md)
- [Flow-through reactions that remove minerals](flow_through.md)
- [Flushing minerals](flush.md)

An simple example input file is:

!listing modules/geochemistry/test/tests/time_dependent_reactions/simple.i
