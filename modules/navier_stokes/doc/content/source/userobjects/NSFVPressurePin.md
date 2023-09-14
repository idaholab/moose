# NSFVPressurePin

!syntax description /UserObjects/NSFVPressurePin

## Overview

The `NSFVPressurePin` can pin the pressure in two modes:

- by offsetting the pressure variable to make it have an average equal to the [!param](/UserObjects/NSFVPressurePin/phi0) parameter value
- by offsetting the pressure variable to make its value equal to the [!param](/UserObjects/NSFVPressurePin/phi0) parameter value in the element
  containing the point specified by the [!param](/UserObjects/NSFVPressurePin/point) parameter.


!alert note
In the [NSFVAction.md], a `NSFVPressurePin` can be used by setting the [!param](/Modules/NavierStokesFV/pinned_pressure_type) parameter
to `average-uo` or `point-value-uo` respectively.

!syntax parameters /UserObjects/NSFVPressurePin

!syntax inputs /UserObjects/NSFVPressurePin

!syntax children /UserObjects/NSFVPressurePin
