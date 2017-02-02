# LevelSetReinitializationMultiApp
The [LevelSetReinitializationMultiApp](#), as the name suggests, is for executing the reinitialization equation for
the level set solution. This [MultiApp](/MultiApps/index.md) object requires that the sub-application be using the
[LevelSetReinitializationProblem](level_set/LevelSetReinitializationProblem.md), which allows for the proper
reseting of the pseudo reinitialization time.

## Example Syntax
!input modules/level_set/tests/reinitialization/master.i block=MultiApps label=False

!parameters /MultiApps/LevelSetReinitializationMultiApp

!inputfiles /MultiApps/LevelSetReinitializationMultiApp

!childobjects /MultiApps/LevelSetReinitializationMultiApp
