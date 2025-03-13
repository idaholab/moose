# PostprocessorDT

!syntax description /Executioner/TimeSteppers/PostprocessorDT

The postprocessor can be computing a global metric, like the Courant Friedrich Levy criterion
with the [CFLTimeStepSize](CFLTimeStepSize.md optional=True) or the
[LevelSetCFLCondition](LevelSetCFLCondition.md optional=true) postprocessor. In those cases,
the postprocessor computes the maximum value of the timestep that should ensure stability.
Variations in numerical schemes may impact the accuracy of this postprocessor, so the
`PostprocessorDT` allows for a scaling factor and an offset to modify the time step.

If the application using the `PostprocessorDT` is a sub-app, its time step may also be
received through a [MultiAppPostprocessorTransfer.md] using a [Receiver.md] postprocessor.

!syntax parameters /Executioner/TimeSteppers/PostprocessorDT

!syntax inputs /Executioner/TimeSteppers/PostprocessorDT

!syntax children /Executioner/TimeSteppers/PostprocessorDT
