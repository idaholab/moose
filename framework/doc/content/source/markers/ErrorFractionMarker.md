# Errorfractionmarker

!syntax description /Adaptivity/Markers/ErrorFractionMarker

## Description

The `ErrorFractionMarker` utilizes the value from an
[Indicator](/Indicators/index.md) as a measure of "error" on each
element. Using this error approximation the following algorithm is
applied:

!media media/markers/error_fraction_marker_example.png style=float:right;width:auto;margin:20px;padding:20px; caption=ErrorFractionMarker example calculation.

1. The elements are sorted by increasing error.
2. The elements comprising the "refine" fraction, from highest error to lowest, of the total error are marked for refinement.
3. The elements comprising the "coarsen" fraction, from lowest error to highest, of the total error are marked for refinement.

## Example Input Syntax

!listing test/tests/markers/error_fraction_marker/error_fraction_marker_test.i block=Adaptivity

!syntax parameters /Adaptivity/Markers/ErrorFractionMarker

!syntax inputs /Adaptivity/Markers/ErrorFractionMarker

!syntax children /Adaptivity/Markers/ErrorFractionMarker
