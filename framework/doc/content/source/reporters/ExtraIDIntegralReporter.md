# ExtraIDIntegralReporter

!syntax description /Reporters/ExtraIDIntegralReporter


## Overview

The `ExtraIDIntegralReporter` object is a reporter object to integrate input variables based on multiple extra element IDs.
This object internally utilizes the vector postprocessor `ExtraIDIntegralVectorPostprocessor` to evaluate the integral values.
Refer to [ExtraIDIntegralVectorPostprocessor](ExtraIDIntegralVectorPostprocessor.md) for details.

## Example Syntax

!listing test/tests/reporters/extra_id_integral/extra_id_integral.i block=Reporters

!syntax parameters /Reporters/ExtraIDIntegralReporter

!syntax inputs /Reporters/ExtraIDIntegralReporter

!syntax children /Reporters/ExtraIDIntegralReporter