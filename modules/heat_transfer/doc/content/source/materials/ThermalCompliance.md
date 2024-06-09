# ThermalCompliance

!syntax description /Materials/ThermalCompliance

## Overview

This material creates a thermal compliance, $K$, material based on temperature gradients and
thermal conductivities using the equation:

!equation
E=\frac{1}{2}\nabla T \dot K \dot \nabla T

The resulting material can be used for analysis purposes and for
setting up topology optimization Solid Isotropic Material Penalization-type of problems.

## Example Input File Syntax

!listing test/tests/optimization/compliance_sensitivity/three_materials_thermal.i block=Materials/thermal_compliance

!syntax parameters /Materials/ThermalCompliance

!syntax inputs /Materials/ThermalCompliance

!syntax children /Materials/ThermalCompliance

!tag name=ThermalCompliance pairs=module:heat_transfer system:materials
