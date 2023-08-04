# Density Update

!syntax description /UserObjects/DensityUpdate

## Description

The `DensityUpdate` class is an `ElementUserObject` that calculates and updates
the design densities based on the filtered sensitivities. The density update is
performed using the Solid Isotropic Material with Penalization (SIMP) method.
The class requires the design density variable, the filtered design density
variable, the density sensitivity variable, the penalty power, and the volume
fraction as inputs.

## Example Input File

An example of how to use the `DensityUpdate` class in an input file:

listing test/tests/materials/ComplianceSensitivity/2d_mbb.i block=UserObjects/update


!syntax parameters /UserObjects/DensityUpdate

!syntax inputs /UserObjects/DensityUpdate

!syntax children /UserObjects/DensityUpdate
