# LevelSetReinitializationProblem
This object specialize the MOOSE problem object to add a custom call to reset the simulation to an initial state so
that the level set reinitialization step can be repeated. The [LevelSetReinitializationMultiApp](level_set/LevelSetReinitializationMultiApp.md) object requires
that the sub-application has a Problem type of [LevelSetReinitializationProblem](#)

## Example Syntax
The [LevelSetReinitializationProblem](#) is invoked by setting the "type" parameter within the [Problem](Problem/index.md) of the
input file.

!listing modules/level_set/tests/transfers/markers/single_level/sub.i block=Problem label=False

!parameters /Problem/LevelSetReinitializationProblem

!inputfiles /Problem/LevelSetReinitializationProblem

!childobjects /Problem/LevelSetReinitializationProblem
