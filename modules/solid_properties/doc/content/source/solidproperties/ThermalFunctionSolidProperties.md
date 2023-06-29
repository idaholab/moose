# ThermalFunctionSolidProperties

!syntax description /SolidProperties/ThermalFunctionSolidProperties

## Description

This userobject provides
thermal properties for an arbitrary solid as a function of temperature.
Parsed function inputs are provided for density, thermal conductivity, and
specific heat by parameterizing the time variable `t` as temperature.
This userobject can also be used to specify constant properties by
excluding any dependence on temperature.

## Range of Validity

The range of validity of this userobject depends on the correlations provided
by the user. Note that arbitrary units can be specified with this userobject
as long as they are consistent with the units of the other objects (e.g. kernels,
boundary conditions, etc.) where the userobject functions are used.

!syntax parameters /SolidProperties/ThermalFunctionSolidProperties

!syntax inputs /SolidProperties/ThermalFunctionSolidProperties

!syntax children /SolidProperties/ThermalFunctionSolidProperties

!bibtex bibliography
