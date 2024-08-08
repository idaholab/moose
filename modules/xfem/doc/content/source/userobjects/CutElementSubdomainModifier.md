# CutElementSubdomainModifier

!syntax description /UserObjects/CutElementSubdomainModifier

## Overview

`CutElementSubdomainModifier` switches the element subdomain ID based on the `CutSubdomainID` marked by geometric cut userobjects. This object extends the capabilities described in [`CoupledVarThresholdElementSubdomainModifier`](meshmodifiers/CoupledVarThresholdElementSubdomainModifier.md) to allow subdomains to be defined based on XFEM interfaces.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/moving_bimaterial_finite_strain_esm.i block=UserObjects

!syntax parameters /MeshModifiers/CutElementSubdomainModifier

!syntax inputs /MeshModifiers/CutElementSubdomainModifier

!syntax children /MeshModifiers/CutElementSubdomainModifier
