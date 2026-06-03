# XYTriangleBoundaryLayerGenerator

!syntax description /Mesh/XYTriangleBoundaryLayerGenerator

## Overview

The `XYTriangleBoundaryLayerGenerator` creates a layered 2D triangulated mesh based on a given surface of a 2D-XY mesh., utilizing the same method that was originally used by [`XYDelaunayGenerator`](/XYDelaunayGenerator.md).

## Input Mesh

The surface can be either provided directly as a 1D mesh representing a closed curve in the XY plane or as a boundary of a given 2D-XY mesh. In both cases, the input mesh is given by [!param](/Mesh/XYTriangleBoundaryLayerGenerator/input). If the input mesh is a 1D mesh, it must only contain a closed curve in the XY plane. If the input mesh is a 2D-XY mesh, it is either a single connected mesh with only one outer boundary manifold, which can be automatically detected, or a mesh with a closed curve boundary that can be specified by [!param](/Mesh/XYTriangleBoundaryLayerGenerator/boundary_names).

## Boundary Layer Specifications

The meshed boundary layer can be generated based on the provided closed curve in either outward or inward normal direction, as determined by the [!param](/Mesh/XYTriangleBoundaryLayerGenerator/boundary_layer_direction) parameter. The thickness of the boundary layer is specified by [!param](/Mesh/XYTriangleBoundaryLayerGenerator/thickness). The number of layers and the bias (i.e., element thickness growth factor) for the layer thickness can be specified by [!param](/Mesh/XYTriangleBoundaryLayerGenerator/num_layers) and [!param](/Mesh/XYTriangleBoundaryLayerGenerator/layer_bias), respectively.

The subdomain ID and name of the meshed boundary layer can be specified by [!param](/Mesh/XYTriangleBoundaryLayerGenerator/subdomain_id) and [!param](/Mesh/XYTriangleBoundaryLayerGenerator/subdomain_name), respectively. If not specified, a trivial subdomain ID of `0` is assigned without a subdomain name. Optionally,
the boundary names of the both interface and surface boundary of the meshed boundary layer can be specified by [!param](/Mesh/XYTriangleBoundaryLayerGenerator/interface_name) and [!param](/Mesh/XYTriangleBoundaryLayerGenerator/surface_name), respectively. If not specified, these boundaries are not created.

If the input mesh is a 2D-XY mesh, it is also optional to keep the input mesh along with the meshed boundary layer by setting [!param](/Mesh/XYTriangleBoundaryLayerGenerator/keep_input) to `true`. In this case, the input mesh and the meshed boundary layer are stitched together to form a single mesh. If not specified, only the meshed boundary layer is generated.

This mesh generator supports both linear and quadratic elements through the [!param](/Mesh/XYTriangleBoundaryLayerGenerator/tri_elem_type) parameter. If not specified, linear elements (`TRI3`) are generated.

## Implementation Details

This mesh generator uses a series of utility methods that enable the existing MOOSE mesh generators to achieve the meshing tasks.

The `XYTriangleBoundaryLayerGenerator` creates a layered 2D triangle mesh along the specified 1D surface of a 2D-XY mesh. It uses several meshing utility methods:
- `MooseMeshUtils::buildPolyLineMesh()`, which is also used by the `GapLineMeshGenerator` , to generate the polyline mesh that defines the layer boundaries of the 2D triangulated mesh to be generated, which need to be parallel to the specified 1D surface of the input mesh.
- `MeshTriangulationUtils::triangulateWithDelaunay`, which was refactored as a utility method from the original algorithm of [`XYDelaunayGenerator`](/XYDelaunayGenerator.md), to generate the 2D triangulated meshes of layers. The generation of each layer uses the two neighboring polyline meshes as the input boundary and hole meshes, respectively.
- `libMesh::UnstructuredMesh::stitch_meshes()`, which is also used by the [`StitchMeshGenerator`](/StitchMeshGenerator.md), to stitch the generated layered 2D triangulated mesh together as well as to stitch the layered 2D triangulated mesh with the input mesh if applicable.

## Examples

Here, two examples of using this mesh generator are illustrated in [coating]. The left example shows the outward boundary layer meshed by this generator based on the outer boundary of a 2D-XY half-circle mesh with its boundary detected automatically, and the right example shows the inward boundary layer meshed by this generator based on a 2D-XY ring mesh with its inner boundary specified.

!media framework/meshgenerators/coating.png
       style=display: block;margin-left:auto;margin-right:auto;width:75%;
       id=coating
       caption=Example of outward (left) and inward (right) boundary layer meshed by this generator. The red blocks are the input meshes, and the grey blocks are the meshed boundary layers.

!syntax parameters /Mesh/XYTriangleBoundaryLayerGenerator

!syntax inputs /Mesh/XYTriangleBoundaryLayerGenerator

!syntax children /Mesh/XYTriangleBoundaryLayerGenerator