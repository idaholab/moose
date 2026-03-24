# BoundaryLayerTriangleGenerator

!syntax description /Mesh/BoundaryLayerTriangleGenerator

## Overview

The `BoundaryLayerTriangleGenerator` creates a layered 2D triangulated mesh utilizing the [`XYDelaunayGenerator`](/XYDelaunayGenerator.md) based on a given surface of a 2D-XY mesh. Aside from `XYDelaunayGenerator`, this mesh generator uses a series of existing MOOSE mesh generator as its sub mesh generators to achieve the meshing tasks, including [`GapLineMeshGenerator`](/GapLineMeshGenerator.md) and [`StitchMeshGenerator`](/StitchMeshGenerator.md). 

## Input Mesh

The surface can be either provided directly as a 1D mesh representing a closed curve in the XY plane or as a boundary of a given 2D-XY mesh. In both case, the input mesh is given by [!param](/Mesh/BoundaryLayerTriangleGenerator/input). If the input mesh is an 1D mesh, it must only contain a closed curve in the XY plane. If the input mesh is a 2D-XY mesh, it is either a single connected mesh with only one outer boundary manifold, which can be automatically detected, or a mesh with a closed curve boundary that can be specified by [!param](/Mesh/BoundaryLayerTriangleGenerator/boundary_names).

## Coating Layer Specifications

The meshed coating layer can be generated based on the provided closed curve in either outward or inward normal direction, as determined by the [!param](/Mesh/BoundaryLayerTriangleGenerator/coating_direction) parameter. The thickness of the coating layer is specified by [!param](/Mesh/BoundaryLayerTriangleGenerator/thickness). The number of layers and the bias (i.e., element thickness growth factor) for the layer thickness can be specified by [!param](/Mesh/BoundaryLayerTriangleGenerator/num_layers) and [!param](/Mesh/BoundaryLayerTriangleGenerator/layer_bias), respectively.

The subdomain ID and name of the meshed coating layer can be specified by [!param](/Mesh/BoundaryLayerTriangleGenerator/subdomain_id) and [!param](/Mesh/BoundaryLayerTriangleGenerator/subdomain_name), respectively. If not specified, a trivial subdomain ID of `0` is assigned without a subdomain name. Optionally,
the boundary names of the both interface and surface boundary of the meshed coating can be specified by [!param](/Mesh/BoundaryLayerTriangleGenerator/interface_name) and [!param](/Mesh/BoundaryLayerTriangleGenerator/surface_name), respectively. If not specified, these boundaries are not created.

If the input mesh is a 2D-XY mesh, it is also optional to keep the input mesh along with the meshed coating layer by setting [!param](/Mesh/BoundaryLayerTriangleGenerator/keep_input) to `true`. In this case, the input mesh and the meshed coating layer are stitched together to form a single mesh. If not specified, only the meshed coating layer is generated.

This mesh generator supports both linear and quadratic elements through the [!param](/Mesh/BoundaryLayerTriangleGenerator/tri_elem_type) parameter. If not specified, linear elements (`TRI3`) are generated.

## Examples

Here, two examples of using this mesh generator are illustrated in [coating]. The left example shows the outward coating meshed by this generator based on the outer boundary of a 2D-XY half-circle mesh with its boundary detected automatically, and the right example shows the inward coating meshed by this generator based on a 2D-XY ring mesh with its inner boundary specified.

!media framework/meshgenerators/coating.png
       style=display: block;margin-left:auto;margin-right:auto;width:75%;
       id=coating
       caption=Example of outward (left) and inward (right) coating meshed by this generator. The red blocks are the input meshes, and the grey blocks are the meshed coating layers.

!syntax parameters /Mesh/BoundaryLayerTriangleGenerator

!syntax inputs /Mesh/BoundaryLayerTriangleGenerator

!syntax children /Mesh/BoundaryLayerTriangleGenerator