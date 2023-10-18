# Compliance Sensitivity

!syntax description /Materials/ComplianceSensitivity

## Description

The `ComplianceSensitivity` material class extends the `StrainEnergyDensity` class to compute the compliance sensitivity in a material. This class is particularly useful in topology optimization problems, where the sensitivity of the compliance to design variables is required. The class makes use of `DerivativeParsedMaterial` to automatically compute derivatives.

## Example Input File

An example of how to use the `ComplianceSensitivity` class in an input file:

listing test/tests/materials/compliance_sensitivity/2d_mbb.i block=Materials/dc


!syntax parameters /Materials/ComplianceSensitivity

!syntax inputs /Materials/ComplianceSensitivity

!syntax children /Materials/ComplianceSensitivity
