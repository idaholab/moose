# Closures1PhaseSimple

!syntax description /Closures/Closures1PhaseSimple

The closures added are:

- a wall friction factor model using [ADWallFrictionFunctionMaterial.md]
- a wall heat transfer coefficient as a [ADWeightedAverageMaterial.md] of the heat transfer coefficients
  defined by the flow channel (deriving from [FlowChannel1Phase.md]) weighted by the heated perimeters of each section.

Additionally, this object defines:

- a wall temperature material, to be able to retrieve the wall temperature as a material property, using either [ADAverageWallTemperature3EqnMaterial.md], AD[CoupledVariableValueMaterial.md] or [ADTemperatureWall3EqnMaterial.md] depending on the temperature mode of the flow channel (deriving from [FlowChannel1Phase.md])


!syntax parameters /Closures/Closures1PhaseSimple

!syntax inputs /Closures/Closures1PhaseSimple

!syntax children /Closures/Closures1PhaseSimple
