# AnalyticalIndicator

!syntax description /Adaptivity/Indicators/AnalyticalIndicator

## Description

The AnalyticalIndicator object computes the difference between a solution variable and
a known function. As the name suggests it is designed for computing the analytical error, but in
practice this indicator is used for debugging and verification applications.

## Example Input File Syntax

The following example demonstrates the creation of an AnalyticalIndicator object within an input
file that uses a [MooseParsedFunction.md] object for computing the known solution.

!listing analytical_indicator_test.i block=Functions Adaptivity

!syntax parameters /Adaptivity/Indicators/AnalyticalIndicator

!syntax inputs /Adaptivity/Indicators/AnalyticalIndicator

!syntax children /Adaptivity/Indicators/AnalyticalIndicator
