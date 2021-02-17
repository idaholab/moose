# GeometricCutElementSubdomainModifier

!syntax description /UserObjects/GeometricCutElementSubdomainModifier

## Overview

`GeometricCutElementSubdomainModifier` switches the element subdomain ID based on the `GeometricCutSubdomainID` marked by geometric cut userobjects. This object is extends the capabilities described in [`CoupledVarThresholdElementSubdomainModifier`](userobject/CoupledVarThresholdElementSubdomainModifier.md) to introduce coupling with XFEM interfaces.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/moving_bimaterial_finite_strain_esm.i block=UserObjects

!syntax parameters /UserObjects/GeometricCutElementSubdomainModifier

!syntax inputs /UserObjects/GeometricCutElementSubdomainModifier

!syntax children /UserObjects/GeometricCutElementSubdomainModifier
