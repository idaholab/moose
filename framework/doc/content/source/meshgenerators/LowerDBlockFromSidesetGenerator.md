# LowerDBlockFromSidesetGenerator

!syntax description /Mesh/LowerDBlockFromSidesetGenerator

## Overview

This mesh generator accepts a list of sidesets and creates a lower dimensional
block of elements along those sides. The order of the newly create lower
dimensional element block will match the order of the parent elements
(e.g. first order for first order parents, second order for second order parents).

Element sides are not explicitly meshed in MOOSE/libMesh, i.e. no shape functions
live on the lower dimensional sides. To run kernels on lower dimensional manifolds
they need to be explicitly meshed. That can be accomplished in a mesh generation
tool like Cubit, or using this MeshGenerator.

In 3D simulations, the appropriate 2D elements (for 2D simulations, 1D elements)
will be meshed in at selected side sets (boundaries).

## Coupling between side and volume meshes

The newly-generated lower dimensional side elements will share nodes with the higher
dimensional volume's elements. Coupling to the variables from volume elements
"just works".

Coupling in the other direction is not as straight forward. The lower dimensional
variables exist on some nodes of the adjacent volume elements. The volume kernels
will run on the volume quadrature points at which *tapered off* values from the
adjacent edges can be found. Use at your own risk.

!syntax parameters /Mesh/LowerDBlockFromSidesetGenerator

!syntax inputs /Mesh/LowerDBlockFromSidesetGenerator

!syntax children /Mesh/LowerDBlockFromSidesetGenerator
