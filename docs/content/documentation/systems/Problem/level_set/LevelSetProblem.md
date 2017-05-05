# LevelSetProblem
This object specialize the MOOSE problem object to add a custom call to the execution of the transfers between
MultiApps allowing for mesh adaptivity from the master application to be applied to the sub-application. This is used
within the level set module to allow the master application to govern the adaptivity of the mesh for a sub-application
that is performing level set reinitialization steps.

## Example Syntax
The [LevelSetProblem](#) is invoked by setting the "type" parameter within the [Problem](Problem/index.md) of the
input file.

!listing modules/level_set/tests/transfers/markers/multi_level/master.i start=[Problem] end=[Executioner] label=False

!parameters /Problem/LevelSetProblem

!inputfiles /Problem/LevelSetProblem

!childobjects /Problem/LevelSetProblem
