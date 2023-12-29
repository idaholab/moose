# ComputeGlobalStrain

!syntax description /Materials/ComputeGlobalStrain

## Description

This `Material` extracts the values from the scalar variables solved by [GlobalStrain](/ScalarKernels/GlobalStrain) and stores them as a strain tensor.
Global strain calculated here is added to the total strain.

!alert warning
Objects that use `use_displaced_mesh = true` will operate on a displaced mesh that does not include the global strain contribution!

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/global_strain/global_strain.i
         block=Materials/global_strain

!syntax parameters /Materials/ComputeGlobalStrain

!syntax inputs /Materials/ComputeGlobalStrain

!syntax children /Materials/ComputeGlobalStrain
