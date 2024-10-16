# PinSurfaceTemperature

!syntax description /Postprocessors/PinSurfaceTemperature

## Overview

<!-- -->

This is a postprocessor that calculates and prints the pin surface temperature `Tpin`.
To be more exact, `Tpin` is calcualted inside the `SCM` solver:

The assumption is that each neighboring subchannel to a specific pin sees the same heat flux.
Using a `Dittus-Boelter` approach we calculate surface temperatures for each neighbor. The numerical
average is `Tpin`.

Then, the postprocessor interpolates the value of the variable `Tpin` at a certain pin and height.

## Example Input File Syntax

!listing /test/tests/problems/SFR/sodium-19pin/test19_monolithic.i block=Postprocessors language=cpp

!syntax parameters /Postprocessors/PinSurfaceTemperature

!syntax inputs /Postprocessors/PinSurfaceTemperature

!syntax children /Postprocessors/PinSurfaceTemperature
