
# ComboMarker
!description /Adaptivity/Markers/ComboMarker

## Description
The `ComboMarker` is used to combine multiple markers into a single marker. This is done by
taking the maximum value of the marker value from the supplied markers. Therefore, refinement
of an element takes precedence.

## Example Input Syntax
!listing test/tests/markers/combo_marker/combo_marker_test.i block=Adaptivity

!parameters /Adaptivity/Markers/ComboMarker

!inputfiles /Adaptivity/Markers/ComboMarker

!childobjects /Adaptivity/Markers/ComboMarker
