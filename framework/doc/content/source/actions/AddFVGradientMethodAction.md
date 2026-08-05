# AddFVGradientMethodAction

!syntax description /FVGradientMethods/AddFVGradientMethodAction

Finite-volume gradient methods are listed inside the `[FVGradientMethods]` block. When MOOSE reads
this block, each named method becomes available to the rest of the input file. Linear
finite-volume variables and gradient-based interpolation methods can then refer to one of these
names when they need cell gradients.

More information about finite-volume gradient methods can be found on the
[FVGradientMethods syntax documentation](syntax/FVGradientMethods/index.md).

!syntax parameters /FVGradientMethods/AddFVGradientMethodAction
