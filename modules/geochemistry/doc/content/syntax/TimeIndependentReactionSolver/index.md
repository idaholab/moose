# TimeIndependentReactionSolver

This sets up a simple time-independent MOOSE simulation, with no spatial dependence and no "usual" solution process (with Kernels, etc).  Apart from setting up a dummy Mesh, Variables, etc, this Action adds a [GeochemistryTimeIndependentReactor](GeochemistryTimeIndependentReactor.md) userobject, many AuxVariables corresponding to molality, free-mg, free-cm3, pH, etc using the [GeochemistryQuantityAux](GeochemistryQuantityAux.md) and a console output object, the [GeochemistryConsoleOutput](GeochemistryConsoleOutput.md).

Here are some simple of examples of where this Action is used in the test suite:

- [Chemical model of seawater](seawater.md)
- [Water in the Amazon river](amazon.md)
- [Red Sea bine](red_sea.md)
- [Morro de Ferro groundwater](morro.md)
- [Microbial respiration](microbial_redox.md)
- [Langmuir sorption of selenate in loamy soil](selenate.md)
- [A problem involving surface complexation](surface_complexation.md)

An example input file is:

!listing modules/geochemistry/test/tests/equilibrium_models/red_sea_precip.i
