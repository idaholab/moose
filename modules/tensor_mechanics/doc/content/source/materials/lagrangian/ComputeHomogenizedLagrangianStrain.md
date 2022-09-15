# ComputeHomogenizedLagrangianStrain

!syntax description /Materials/ComputeHomogenizedLagrangianStrain

## Overview

This simple material is part of the [Lagrangian kernel homogenization system](Homogenization.md).
It extracts the values of the homogenization strain or deformation
gradient from a `ScalarVariable` and defines the appropriate gradient
tensor.  The [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md) then
adds this extra gradient to the deformation gradient calculated from
the displacement field.
The [homogenization system](Homogenization.md) documentation describes how the
strain/gradient components are stored in the `ScalarVariable`.

The [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md) can add this object
automatically, which is the recommended way to set up homogenization constraints.

## Example Input File Syntax

This example manually specifies the parameters required to initialize the object for a
large deformation, 3D example.
The important input parameters are `macro_gradient`, the name of the `ScalarVariable`,
and `large_kinematics` which determines if the `ScalarVariable` holds a symmetric
small strain tensor (`false`) or a non-symmetric displacement gradient (`large`).

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/total/homogenization/large-tests/3d.i
         block=Materials

!syntax parameters /Materials/ComputeHomogenizedLagrangianStrain

!syntax inputs /Materials/ComputeHomogenizedLagrangianStrain

!syntax children /Materials/ComputeHomogenizedLagrangianStrain
