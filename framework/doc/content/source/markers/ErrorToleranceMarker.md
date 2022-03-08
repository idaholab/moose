# ErrorToleranceMarker

!syntax description /Adaptivity/Markers/ErrorToleranceMarker

## Description

The `ErrorTolerenceMarker` requires an approximation of
the error to be supplied via an
[Indicator](/Indicators/index.md). Using the measure of error an
element is define for coarsening or refinement using a hard tolerances:

* Refine if the element error is greater than value supplied in "refine" input parameter.
* Coarsen if the element error is less than value supplied in "coarsen" input parameter.

## Example Input Syntax

!listing test/tests/markers/error_tolerance_marker/error_tolerance_marker_test.i block=Adaptivity

!syntax parameters /Adaptivity/Markers/ErrorToleranceMarker

!syntax inputs /Adaptivity/Markers/ErrorToleranceMarker

!syntax children /Adaptivity/Markers/ErrorToleranceMarker
