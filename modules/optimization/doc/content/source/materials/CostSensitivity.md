# CostSensitivity

!syntax description /Materials/CostSensitivity

## Overview

This material takes a [`DerivativeParsedMaterial`](/DerivativeParsedMaterial.md) and uses its
derivative with respect to a variable. Such a variable can be a pseudo-density which is
employed to solve a solid isotropic material penalization (SIMP) topology optimization
problem.

## Example Input File Syntax

This material can be used to optimize a multimaterial topology problem, in which the
constraints are not only based on volume but also on an additional cost function.

!listing test/tests/optimization/compliance_sensitivity/paper_three_materials_test.i block=Materials/cc

!syntax parameters /Materials/CostSensitivity

!syntax inputs /Materials/CostSensitivity

!syntax children /Materials/CostSensitivity

!tag name=CostSensitivity pairs=module:optimization system:materials
