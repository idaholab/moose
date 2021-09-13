# SetupQuadratureAction

!syntax description /Executioner/Quadrature/SetupQuadratureAction

A `Quadrature` is specified as an object inside the `[Quadrature]` block with the `[Executioner]` block
as shown below.

!listing test/tests/quadrature/order/block-order.i block=Executioner

This action adds the default quadrature rule to the `Problem` then adds custom requested quadratures
in the specified blocks.

More information about quadratures may be found on the
[Quadrature syntax documentation](syntax/Executioner/Quadrature/index.md).

!syntax parameters /Executioner/Quadrature/SetupQuadratureAction
