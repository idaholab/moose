# AddFVInterpolationMethodAction

!syntax description /FVInterpolationMethods/AddFVInterpolationMethodAction

Finite volume interpolation objects are specified as objects inside the `[FVInterpolationMethods]` block.
This action adds them to the [Problem](syntax/Problem/index.md). The main responsibility of
these kernels is to compute face values (or face value contributions to a system) from cell center values.

More information about linear finite volume kernels can be found on the
[FVInterpolationMethods syntax documentation](syntax/FVInterpolationMethods/index.md).

!syntax parameters /FVInterpolationMethods/AddFVInterpolationMethodAction
