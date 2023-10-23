# Density Sensitivities

!syntax description /UserObjects/SensitivityFilter

## Description

The `SensitivityFilter` class is an `ElementUserObject` that calculates the density sensitivities
for topology optimization problems. It uses a radial average filter to distribute the
sensitivities across elements. The class requires the `RadialAverage` user object,
the density sensitivity variable, and the design density variable as inputs.

In general, the radius for filtering the sensitivity variables should be selected that
numerical instabilities are avoided but good spatial resolution is obtained. Typically,
selecting a radius value such that it encompasses a few finite elements works best.
## Example Input File

An example of how to use the `SensitivityFilter` class in an input file:

listing test/tests/materials/compliance_sensitivity/2d_mbb.i block=UserObjects/calc_sense


!syntax parameters /UserObjects/SensitivityFilter

!syntax inputs /UserObjects/SensitivityFilter

!syntax children /UserObjects/SensitivityFilter
