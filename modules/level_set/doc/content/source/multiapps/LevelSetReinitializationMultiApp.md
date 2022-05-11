# LevelSetReinitializationMultiApp

The LevelSetReinitializationMultiApp, as the name suggests, is for executing the reinitialization
equation for the level set solution. This [MultiApp](/MultiApps/index.md) object requires that the
sub-application be using the [LevelSetReinitializationProblem](/LevelSetReinitializationProblem.md),
which allows for the proper resetting of the pseudo reinitialization time.

## Example Syntax

!listing modules/level_set/test/tests/reinitialization/parent.i block=MultiApps

!syntax parameters /MultiApps/LevelSetReinitializationMultiApp

!syntax inputs /MultiApps/LevelSetReinitializationMultiApp

!syntax children /MultiApps/LevelSetReinitializationMultiApp
