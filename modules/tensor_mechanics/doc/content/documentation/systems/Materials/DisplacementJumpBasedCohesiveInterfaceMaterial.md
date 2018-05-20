# DisplacementJumpBasedCohesiveInterfaceMaterial

<!-- !syntax description /Materials/DisplacementJumpBasedCohesiveInterfaceMaterial -->

This class implement a general Cohesive Interface Material and does all the computation to produce residual and jacobian coefficients used by the `DisplacementJumpBasedCohesiveInterfaceKernel`

## Example Input File Syntax

Required parameters are the variables representing the displacement field on both side of the interface, and the userobject in which the cohesive law is implemented.


### Single interface 2D

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D.i block=Materials/gap

### Single interface 3D

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/3D/czmTest3DC_CohesiveLaw3D.i block=Materials/gap

### Multiple interfaces

When `split_interface=true` the cohesive interface is split by block pairs and one material block must be included for each userobeject

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D_splitInterface.i block=Materials/interface_12_13

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D_splitInterface.i block=Materials/interface_24_34

<!-- !listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D_splitInterface.i  -->



!syntax parameters /Materials/DisplacementJumpBasedCohesiveInterfaceMaterial

!syntax inputs /Materials/DisplacementJumpBasedCohesiveInterfaceMaterial
