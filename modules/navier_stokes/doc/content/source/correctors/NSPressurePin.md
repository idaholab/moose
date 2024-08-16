# NSPressurePin

!syntax description /Correctors/NSPressurePin

## Overview

The `NSPressurePin` can pin the pressure in two modes:

- by offsetting the pressure variable to make it have an average equal to the [!param](/Correctors/NSPressurePin/phi0) parameter value
- by offsetting the pressure variable to make its value equal to the [!param](/Correctors/NSPressurePin/phi0) parameter value in the element
  containing the point specified by the [!param](/Correctors/NSPressurePin/point) parameter.


!alert note
In the [NSFVAction.md], a `NSPressurePin` can be used by setting the [!param](/Modules/NavierStokesFV/pinned_pressure_type) parameter
to `average-uo` or `point-value-uo` respectively.

!syntax parameters /Correctors/NSPressurePin

!syntax inputs /Correctors/NSPressurePin

!syntax children /Correctors/NSPressurePin
