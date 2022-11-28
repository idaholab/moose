# IterationInfo

A Reporter object for tracking iteration information, such as the number of linear and nonlinear
iterations. The [!param](/Reporters/IterationInfo/items) parameter controls the
items computed. `IterationInfo` will declare reporters for each item requested.

## Example Input Syntax

The following input file snippet demonstrates the use of the `IterationInfo` object.

!listing iteration_info/iteration_info.i block=Reporters/iteration_info


!syntax parameters /Reporters/IterationInfo

!syntax inputs /Reporters/IterationInfo

!syntax children /Reporters/IterationInfo
