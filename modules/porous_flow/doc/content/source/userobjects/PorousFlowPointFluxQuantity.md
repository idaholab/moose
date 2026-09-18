# PorousFlowPointFluxQuantity

This records the instantaneous flux (kg.s$^{-1}$ for fluid, J.s$^{-1}$ for heat) at each
individual Dirac point of a PorousFlow line sink, such as
[`PorousFlowPeacemanBorehole`](PorousFlowPeacemanBorehole.md) or
[`PorousFlowPolyLineSink`](PorousFlowPolyLineSink.md), as computed during the most recent
residual evaluation.  Unlike [`PorousFlowSumQuantity`](PorousFlowSumQuantity.md), which records
the total mass/heat extracted over an entire line sink during a time step, this records the
per-point flow rate, which is useful for visualising the flow profile along a wellbore.  Wire a
`PorousFlowPointFluxQuantity` into a line sink's `PointFluxUO` parameter, and read it out with
[`PorousFlowPlotPointFluxQuantity`](PorousFlowPlotPointFluxQuantity.md).  See
[polyline sinks](sinks.md) for an extended discussion.

!alert note
Use a separate `PorousFlowPointFluxQuantity` for each line sink: sharing one between multiple
line sinks will cause each one's `zero()` call to erase the others' recorded values.

!syntax parameters /UserObjects/PorousFlowPointFluxQuantity

!syntax inputs /UserObjects/PorousFlowPointFluxQuantity

!syntax children /UserObjects/PorousFlowPointFluxQuantity
