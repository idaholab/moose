# ComputeBlockOrientationByMisorientation

In a crystal plasticity simulation, the `ComputeBlockOrientationByMisorientation` user object determines the orientation of each grain, represented by material blocks (subdomains), by identifying the material point orientation that undergoes the most rotation during the preceding time step. It accomplishes this by calculating the maximum misorientation within each material block and selecting the orientation that yields the highest misorientation value.

The misorientation value, denoted as $M$, is computed in the [ComputeMultipleCrystalPlasticityStress](ComputeMultipleCrystalPlasticityStress.md) class using the formula: $M = \arccos(\left(\text{trace}(\boldsymbol{M}) - 1\right) / 2)$, where $\boldsymbol{M} = \boldsymbol{R} * \boldsymbol{R}\text{old}^{-1}$ represents the misorientation matrix, and $\boldsymbol{R}$ and $\boldsymbol{R}\text{old}$ are the current and previous time step's rotation matrices, respectively. Please refer to [wiki](https://en.wikipedia.org/wiki/Misorientation#:~:text=Misorientation%20distribution,-Example%20MDF%20shown&text=The%20MD%20can%20be%20calculated,to%20a%20bin%20and%20accumulated.) for more details information regarding the misorientation value calulcation.

Alternatively, an additional method for calculating block orientation can be found in [ComputeBlockOrientationByRotation](ComputeBlockOrientationByRotation.md).

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/block_orientation/block_orientation_calculation.i block=UserObjects/block_orientation

!syntax parameters /UserObjects/ComputeBlockOrientationByMisorientation

!syntax inputs /UserObjects/ComputeBlockOrientationByMisorientation

!syntax children /UserObjects/ComputeBlockOrientationByMisorientation
