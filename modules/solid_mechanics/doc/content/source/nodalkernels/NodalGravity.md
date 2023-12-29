# Nodal Gravity

!syntax description /NodalKernels/NodalGravity

# Description

This NodalKernel computes the gravitational force proportional to nodal mass in the coordinate direction corresponding to the assigned variable. A constant gravitational acceleration can be provided using `gravity_value` parameter and an acceleration varying in space and time can be provided using the `function` parameter. A constant mass for all the nodes in the given boundary can be provided using the `mass` parameter. Otherwise, a CSV file containing nodal positions and the corresponding nodal masses can also be provided using the `nodal_mass_file` parameter.

For example, the below csv file has two rows with 4 columns. The first three columns correspond to the nodal positions in the global coordinate system and the last column corresponds to the nodal mass. Each row contains position and mass information for one node.

!listing modules/tensor_mechanics/test/tests/beam/dynamic/nodal_mass.csv

!syntax parameters /NodalKernels/NodalGravity

!syntax inputs /NodalKernels/NodalGravity

!syntax children /NodalKernels/NodalGravity
