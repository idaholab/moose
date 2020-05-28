# EquilibriumReactionSolverOutput

Usually this Output is added using the [EquilibriumReactionSolver action](EquilibriumReactionSolver/index.md).  The purpose of this Output is to solve an equilibrium reaction system and then output bulk compositions, molalities, activities, etc.  Some examples are:

- [Chemical model of seawater](tests_and_examples/seawater.md) which includes a model interacting with external gas buffers and mineral precipitation.

- [Chemical model of amazon river water](tests_and_examples/amazon.md) which includes fixing the pH, preventing precipitation, and fixing free mineral volumes

- [Chemical model of Red Sea brine](tests_and_examples/red_sea.md) which includes fixing the pH, high-temnperature analysis, fixing free mineral volumes, and computing saturation indices

- [Chemical models of Morro de Ferro groundwater](tests_and_examples/morro.md) which includes redox disequilibrium and analysis of Nernst potentials

- [Microbial respiration](tests_and_examples/microbial_redox.md) which includes redox disequilibrium and analysis of Nernst potentials

- [Sorption of selenate](tests_and_examples/selenate.md) which includes redox disequilibrium and sorption via the Langmuir model

- [Sorption onto ferric hydroxide](tests_and_examples/surface_complexation.md) which includes surface complexation

!syntax parameters /Outputs/EquilibriumReactionSolverOutput

!syntax inputs /Outputs/EquilibriumReactionSolverOutput

!syntax children /Outputs/EquilibriumReactionSolverOutput
