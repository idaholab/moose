# SetupQuadratureAction

!syntax description /Executioner/Quadrature/SetupQuadratureAction

A `Quadrature` is specified as an object inside the `[Quadrature]` block with the `[Executioner]` block
as shown below.

!listing test/tests/quadrature/order/block-order.i block=Executioner

This action adds the default quadrature rule to the `Problem` then adds custom requested quadratures
in the specified blocks.

## Per-block quadrature types

In addition to specifying custom quadrature orders per block via `custom_orders`, it is also possible
to specify a custom quadrature +type+ per block using the `custom_types` parameter. The `custom_types`
list must have the same number of entries as `custom_blocks`. If `custom_types` is omitted, the global
`type` is used for all custom blocks, preserving existing behaviour.

The following example specifies both a custom order and a custom quadrature type for two subdomains:
```text
[Executioner]
  [Quadrature]
    type = GAUSS
    order = THIRD
    custom_blocks    = '2 3'
    custom_orders    = 'THIRD SECOND'
    custom_types     = 'GAUSS_LOBATTO SIMPSON'
  []
[]
```

More information about quadratures may be found on the
[Quadrature syntax documentation](syntax/Executioner/Quadrature/index.md).

!syntax parameters /Executioner/Quadrature/SetupQuadratureAction
```