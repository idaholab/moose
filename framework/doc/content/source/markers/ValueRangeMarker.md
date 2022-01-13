# ValueRangeMarker

!syntax description /Adaptivity/Markers/ValueRangeMarker

## Description

The `ValueRangeMarker` utilizes an
[Indicator](/Indicators/index.md) as en estimate of error. Elements
with error estimates inside and outside of an allowable range can then
be marked for coarsening or refinement.

By default values inside the range are marked for refinement, but this
may be reversed using the "invert" parameter.

Additionally, a third state buffer region exists outside of the define
range bounds. Elements in this region may be marked to a different
state. Typically, this buffer region is used to define a region of
elements that are marked with "DO_NOTHING" to avoid having elements
marked for refinement directly adjacent to elements marked for
refinement.

## Example Input Syntax

!listing test/tests/markers/value_range_marker/value_range_marker_test.i block=Adaptivity

!syntax parameters /Adaptivity/Markers/ValueRangeMarker

!syntax inputs /Adaptivity/Markers/ValueRangeMarker

!syntax children /Adaptivity/Markers/ValueRangeMarker
