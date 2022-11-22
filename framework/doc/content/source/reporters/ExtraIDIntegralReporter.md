# ExtraIDIntegralReporter

!syntax description /Reporters/ExtraIDIntegralReporter


## Overview

The `ExtraIDIntegralReporter` object is a reporter object to integrate input variables based on multiple extra element IDs.
This object internally utilizes the vector postprocessor `ExtraIDIntegralVectorPostprocessor` to evaluate the integral values.
Refer to [ExtraIDIntegralVectorPostprocessor](ExtraIDIntegralVectorPostprocessor.md) for details.

!alert note title=Reporter names
The reporters created by the `ExtraIDIntegralReporter` are the `extra_id_data`, holding the [!param](/Reporters/ExtraIDIntegralReporter/id_name)
requested and map from names to indexes into the integrals, and the `integrals`, holding the integral values.

## Example Syntax

!listing test/tests/reporters/extra_id_integral/extra_id_integral.i block=Reporters

!syntax parameters /Reporters/ExtraIDIntegralReporter

!syntax inputs /Reporters/ExtraIDIntegralReporter

!syntax children /Reporters/ExtraIDIntegralReporter