# ComputeBlockOrientationByRotation

In a crystal plasticity simulation, the `ComputeBlockOrientationByRotation`  user object calculates the orientation of each grain, represented by material blocks (subdomains), by selecting the most prevalent orientation direction among all material points within each block. It achieves this by:

1. Converting the Euler angles of every element within the block into quaternions.
2. Computing the distribution of quaternion values.
3. Choosing the most prevalent orientation direction as the grain orientation.

A alternative way of calculating the block orientation can be found in [ComputeBlockOrientationByMisorientation](ComputeBlockOrientationByMisorientation.md).

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/block_orientation/block_orientation_calculation.i block=UserObjects/block_orientation

!syntax parameters /UserObjects/ComputeBlockOrientationByRotation

!syntax inputs /UserObjects/ComputeBlockOrientationByRotation

!syntax children /UserObjects/ComputeBlockOrientationByRotation
