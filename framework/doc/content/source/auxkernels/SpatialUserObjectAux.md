# SpatialUserObjectAux

!syntax description /AuxKernels/SpatialUserObjectAux

User objects are generally more flexible than auxiliary kernels, and this object creates
allows to connect them to the [`Variables` system](syntax/Variables/index.md).

The `UserObject` _must_ implement the `spatialValue()` virtual function. In the framework,
the following objects can currently be used:

- [PostprocessorSpatialUserObject.md]

- [NearestPointIntegralVariablePostprocessor.md]

- LayeredIntegral-derived objects such as [LayeredAverage.md] and [LayeredIntegral.md]

- LayeredSideIntegral-derived objects such as [LayeredSideAverage.md] and [LayeredSideIntegral.md]

- NearestPointBase-derived objects such as [NearestPointLayeredAverage.md]

- [LineValueSampler.md]

- [FXIntegralBaseUserObject](FXIntegralBaseUserObject.md optional=True)


Additional objects may be implemented in an application.

## Example syntax

In this example, a `SpatialUserObjectAux` is being used to store in an auxiliary variable the
averages of another variable, u, on multiple layers along the `right` boundary in the `y` direction.

!listing test/tests/userobjects/layered_side_integral/layered_side_average.i block=AuxKernels UserObjects

!alert note
Automatic differentiation is not supported in the auxiliary variable system, so using a `SpatialUserObjectAux`
will not propagate derivatives.

!syntax parameters /AuxKernels/SpatialUserObjectAux

!syntax inputs /AuxKernels/SpatialUserObjectAux

!syntax children /AuxKernels/SpatialUserObjectAux
