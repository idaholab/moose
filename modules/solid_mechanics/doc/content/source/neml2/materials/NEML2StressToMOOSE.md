# NEML2StressToMOOSE

!syntax description /Materials/NEML2StressToMOOSE

## Description

This object retrieves both the MOOSE `small_stress` and `small_jacobian` values from the NEML2 model output computed through [ExecuteNEML2Model](ExecuteNEML2Model.md). In addition to just retrieving a NEML2 output and its derivative, this object also performs an [objective integration](ComputeLagrangianObjectiveStress.md), making both the Cauchy and PK1 stresses available.

## Example Input Syntax

!syntax parameters /Materials/NEML2StressToMOOSE

!syntax inputs /Materials/NEML2StressToMOOSE

!syntax children /Materials/NEML2StressToMOOSE
