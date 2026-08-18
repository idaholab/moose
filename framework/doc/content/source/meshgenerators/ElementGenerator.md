# ElementGenerator

!syntax description /Mesh/ElementGenerator

## Overview

This MeshGenerator object allows the user to create a single element (it is possible to create a simple mesh by adding elements one by one).

The user has to provide the positions of the nodes for their element, as well as the type of element they want (QUAD4, TRI3,...) and the element node connectivity.

Multiple `ElementGenerator` objects may be chained through the `input` parameter to add
elements one at a time. Each generator adds the nodes it is given; if chained elements
use the same coordinates and should be topologically connected, pass the result through
a mesh generator that merges coincident nodes, such as `MeshRepairGenerator` with
[!param](/Mesh/MeshRepairGenerator/fix_node_overlap) set to `true`.

!alert note
The reader is referred to the libmesh documentation and [doxygen](https://mooseframework.inl.gov/docs/doxygen/libmesh/) for the
conventions on node connectivity in a type of element to create a well formed, positive-volume, non self-intersecting, element.

## Further ElementGenerator Documentation

!syntax parameters /Mesh/ElementGenerator

!syntax inputs /Mesh/ElementGenerator

!syntax children /Mesh/ElementGenerator
