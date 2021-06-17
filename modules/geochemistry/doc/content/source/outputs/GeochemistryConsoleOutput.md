# GeochemistryConsoleOutput

This Output writes information to the console regarding the geochemical system (defined by the `geochemistry_reactor`) at a particular `point` that is defined by a [NearestNodeNumber UserObject](NearestNodeNumberUO.md).  It is quite verbose since it provides information about molalities, free masses, bulk compositions, surface characteristics and Nernst information for all relevant species, as well as descriptive characteristics such as temperature, pH, and ionic strength.  It should therefore be used with caution otherwise your std::out will rapidly fill up, but it is useful for understanding and debugging models.

Usually this Output is added using Actions such as the [TimeIndependentReactionSolver](TimeIndependentReactionSolver/index.md) and the [TimeDependentReactionSolver](TimeDependentReactionSolver/index.md).

Advanced users may wish to add these objects manually to their input files.  Here is an example:

!listing test/tests/geochemistry_console_output/console.i block=Outputs

!syntax parameters /Outputs/GeochemistryConsoleOutput

!syntax inputs /Outputs/GeochemistryConsoleOutput

!syntax children /Outputs/GeochemistryConsoleOutput
