# PorousFlowPlotPointFluxQuantity

This extracts the flux, and the coordinates, at each point of a PorousFlow line sink from a
[`PorousFlowPointFluxQuantity`](PorousFlowPointFluxQuantity.md) UserObject, and outputs them as
a table with columns `point_id`, `x`, `y`, `z` and `flux`, one row per point of the line sink.
See [polyline sinks](sinks.md) for an extended discussion.

!alert note
The reported flux at each point is summed over the test functions of the element containing
that point, so when nodal multiplicative factors (such as `use_mobility`) are active on the
line sink, the reported value is a test-function-weighted combination of those nodal values,
rather than a single nodal value.

!syntax parameters /VectorPostprocessors/PorousFlowPlotPointFluxQuantity

!syntax inputs /VectorPostprocessors/PorousFlowPlotPointFluxQuantity

!syntax children /VectorPostprocessors/PorousFlowPlotPointFluxQuantity
