# Compliance Sensitivity

!syntax description /Materials/ComplianceSensitivity

## Description

The `ComplianceSensitivity` material class extends the `StrainEnergyDensity` class to compute the compliance sensitivity in a material. This class is particularly useful in topology optimization problems, where the sensitivity of the compliance to design variables is required. The class takes into account the design density, the penalty power for the SIMP method, and Young's modulus values for the material.

## Example Input File

An example of how to use the `ComplianceSensitivity` class in an input file:



!syntax parameters /Materials/ComplianceSensitivity

!syntax inputs /Materials/ComplianceSensitivity

!syntax children /Materials/ComplianceSensitivity
