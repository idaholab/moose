# ThermalSensitivity

!syntax description /Materials/ThermalSensitivity

## Overview

TThis material takes a [`DerivativeParsedMaterial`](/DerivativeParsedMaterial.md) and uses its
derivative with respect to the [!param](/Materials/ThermalSensitivity/design_density) variable.
The 'design_density' can be a pseudo-density which is
employed to solve a solid isotropic material penalization (SIMP) topology optimization
problem. This sensitivity is intended to optimize a thermal compliance objective function.

## Example Input File Syntax

This material can be used to optimize a multimaterial topology problem or in combination with
other physics, such as a small deformation elastic problem

!listing test/tests/optimization/compliance_sensitivity/three_materials_thermal.i block=Materials/tc

!syntax parameters /Materials/ThermalSensitivity

!syntax inputs /Materials/ThermalSensitivity

!syntax children /Materials/ThermalSensitivity

!tag name=ThermalSensitivity pairs=module:heat_transfer system:materials
