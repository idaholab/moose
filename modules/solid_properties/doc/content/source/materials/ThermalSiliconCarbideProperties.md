# ThermalSiliconCarbideProperties

!syntax description /Materials/ThermalSiliconCarbideProperties

## Description

`ThermalSiliconCarbideProperties` provides thermal properties
for monolithic silicon carbide as a function of temperature.

Isobaric specific heat is calculated from [cite:snead].
Two methods are available for computing thermal conductivity because there is
a very wide variation in thermal conductivity values reported for silicon
carbide. The first correlation is from [cite:snead], and generally
applies for chemical vapor deposition silicon carbide. The second correlation
is from [cite:parfume] and has been used for modeling the same types of
materials as the first correlation, but generally predicts thermal conductivities
about two times smaller than the Snead correlation.
The density is assumed constant with a default value provided as an average
over four different crystal structures at room temperature [cite:snead].
Molar mass is calculated assuming a 1:1 mixture of pure elemental silicon and carbon.

## Range of Validity

The ThermalSiliconCarbideProperties Material is valid for estimating isobaric
specific heat over 200 K $\le$ T $\le$ 2400 K; for estimating thermal
conductivity over 300 K $\le$ T $\le$ 1800 K with the Snead correlation
[cite:snead] and over an unspecified range for the second correlation
[cite:parfume].

## Example Input File Syntax

!listing modules/solid_properties/test/tests/silicon_carbide/test.i block=Materials

!syntax parameters /Materials/ThermalSiliconCarbideProperties

!syntax inputs /Materials/ThermalSiliconCarbideProperties

!syntax children /Materials/ThermalSiliconCarbideProperties

!bibtex bibliography
