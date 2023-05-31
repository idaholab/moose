# Density Sensitivities

!syntax description /UserObjects/SensitivityFilter

## Description

The `SensitivityFilter` class is an `ElementUserObject` that calculates the density sensitivities for topology optimization problems. It uses a radial average filter to distribute the sensitivities across elements. The class requires the `RadialAverage` user object, the density sensitivity variable, and the design density variable as inputs.

## Example Input File

An example of how to use the `SensitivityFilter` class in an input file:




!syntax parameters /UserObjects/SensitivityFilter

!syntax inputs /UserObjects/SensitivityFilter

!syntax children /UserObjects/SensitivityFilter
