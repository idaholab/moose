# DualMeshGenerator

!syntax description /Mesh/DualMeshGenerator

## Overview

The dual mesh generator is a tool for improving the fidelity of meshes for CFD. The dual mesh generator creates a Voronoi dual of a primal mesh with nodes positioned at the centroids of each primal element, midpoints on the primal mesh boundary sides, and boundary vertices of the primal mesh. Currently only supports 2D input.


Through the `boundary_node_angular_tol` option it is possible to specify the angular tolerance (in degrees) of the colinearity test that identifies vertices of the input mesh. Default: 1

!syntax parameters /Mesh/DualMeshGenerator

!syntax inputs /Mesh/DualMeshGenerator

!syntax children /Mesh/DualMeshGenerator
