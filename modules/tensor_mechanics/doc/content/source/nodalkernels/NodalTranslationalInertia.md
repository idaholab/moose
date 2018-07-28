# Nodal Translational Inertia

!syntax description /NodalKernels/NodalTranslationalInertia

# Description

This NodalKernel computes the $i^{th}$ component of translational inertial force proportional to nodal mass. Mass proportional Rayleigh damping is also computed by this NodalKernel. A constant mass for all the nodes in the given boundary can be provided using the `mass` parameter. Otherwise, a CSV file containing nodal positions and the corresponding nodal masses can also be provided using the `nodal_mass_file` parameter. Please refer to [C0TimoshenkoBeam](/C0TimoshenkoBeam.md) for details.

For example, the below csv file has two rows with 4 columns. The first three columns correspond to the nodal positions in the global coordinate system and the last column corresponds to the nodal mass. Each row contains position and mass information for one node.

!listing modules/tensor_mechanics/test/tests/beam/dynamic/nodal_mass.csv


!syntax parameters /NodalKernels/NodalTranslationalInertia

!syntax inputs /NodalKernels/NodalTranslationalInertia

!syntax children /NodalKernels/NodalTranslationalInertia
