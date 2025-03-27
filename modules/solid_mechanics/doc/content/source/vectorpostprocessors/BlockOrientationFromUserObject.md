# BlockOrientationFromUserObject

The `BlockOrientationFromUserObject` vector postprocessor collects Euler angle (degrees) information for each material block from a user object, such as [ComputeBlockOrientationByRotation](ComputeBlockOrientationByRotation.md) or [ComputeBlockOrientationByMisorientation](ComputeBlockOrientationByMisorientation.md), and structures the data into four columns:i.e., `subdomain_id`, `euler_angle_0`, `euler_angle_1` and `euler_angle_2`.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/block_orientation/block_orientation_calculation.i block=VectorPostprocessors/block_ea

!syntax parameters /VectorPostprocessors/BlockOrientationFromUserObject

!syntax inputs /VectorPostprocessors/BlockOrientationFromUserObject

!syntax children /VectorPostprocessors/BlockOrientationFromUserObject
