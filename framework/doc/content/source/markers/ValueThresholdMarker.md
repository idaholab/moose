# ValueThresholdMarker

!syntax description /Adaptivity/Markers/ValueThresholdMarker

## Description

The `ValueThresholdMarker` requires an estimate of error
be provided from an [Indicator](/Indicators/index.md). Using this
indicator an element is marked for refinement or coarsening if the
element error value above or below the coarsening and refinement
thresholds.

By default elements with error greater than the threshold defined in
the "refine" parameter will be marked for refinement. Element with
error less than the threshold defined in the "coarsen" parameters will
be marked for coarsening. This behavior can be inverted using the
"invert" parameter.

Additionally, a third buffer region can be defined. Elements in this
region may be marked to a different state. Typically, this buffer
region is used to define a region of elements that are marked with
"DO_NOTHING" to avoid having elements marked for refinement directly
adjacent to elements marked for refinement.

## Example Input Syntax

!listing test/tests/markers/value_threshold_marker/value_threshold_marker_test.i block=Adaptivity

!syntax parameters /Adaptivity/Markers/ValueThresholdMarker

!syntax inputs /Adaptivity/Markers/ValueThresholdMarker

!syntax children /Adaptivity/Markers/ValueThresholdMarker
