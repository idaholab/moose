# MeshDiagnosticsGenerator

!syntax description /Mesh/MeshDiagnosticsGenerator

!alert note
This mesh generator currently will only accept serialized meshes.  This is most easily accomplished by leaving the mesh type set to `replicated`.
Some of the diagnostics would work as well for distributed meshes, contributions are welcome.

The diagnostics implemented are presented below. The filters and visualization capabilities in Paraview or
other visualization software may be leveraged for further diagnostics of mesh issues.

## Element volume check

Turn this check on with the [!param](/Mesh/MeshDiagnosticsGenerator/examine_element_volumes) parameter.

A minimum and a maximum element volume can be specified to the `MeshDiagnosticsGenerator` using
the [!param](/Mesh/MeshDiagnosticsGenerator/minimum_element_volumes) and the
[!param](/Mesh/MeshDiagnosticsGenerator/maximum_element_volumes) parameters.
These diagnostics are mostly used to detect zero or negative volume elements, from distorted meshes.

!alert note title=How to fix large elements
Large elements may be sub-divided using the [RefineBlockGenerator.md] with for example a
[!param](/Mesh/RefineBlockGenerator/max_element_volume).

!alert note title=How to fix negative volume elements
Negative volume elements can be flipped using a [MeshRepairGenerator.md] with the
[!param](/Mesh/MeshRepairGenerator/fix_elements_orientation) parameter. This will flip them, making the volume
positive, though it may not fix the Jacobian everywhere.

## Side planarity check

Turn this check on with the [!param](/Mesh/MeshDiagnosticsGenerator/examine_nonplanar_sides) parameter.

Non-planar elements are generally *supported* in MOOSE. However, the ray-tracing capabilities treat them
approximately at the moment, so it may be useful to examine the mesh before use.

## Subdomain element types check

Turn this check on with the [!param](/Mesh/MeshDiagnosticsGenerator/examine_element_types) parameter.

The [Exodus.md] output format only supports a single element type (TRI3, QUAD4, HEX8, HEX20, etc) per
subdomain. Rather than erroring at the time of output, this diagnostic outputs which subdomains present
multiple element types.

!alert note title=How to fix subdomains with mixed element types
Subdomains with mixed element types can be split by element type using a [MeshRepairGenerator.md] with the
[!param](/Mesh/MeshRepairGenerator/separate_blocks_by_element_types) parameter.

## Sideset orientation check

Turn this check on with the [!param](/Mesh/MeshDiagnosticsGenerator/examine_sidesets_orientation) parameter.

Sidesets are oriented. They are represented in MOOSE by a list of elements and sides. Along the boundary,
the orientation of these sides should in general be consistent. If it is not, the normal of the sideset is
essentially reversed from one side to the next in the sideset.

The sideset orientation check is a heuristic for a common error case.
The sideset orientation is only checked for sides that are delimiting two different subdomains. The check
makes sure that the order of the subdomains, e.g. which one owns the element and which one owns the element on the
other side of the side (the neighbor), does not change throughout the sideset.

## Element overlap checks

Turn this check on with the [!param](/Mesh/MeshDiagnosticsGenerator/examine_element_overlap) parameter.

Overlap between elements are generally undesirable. Overlapping elements can be associated with non-conformality and/or
breaks in the mesh, which generally are not supported without special numerical treatments, such as using
[interface kernels](syntax/InterfaceKernels/index.md) or mortar methods. Beyond having disconnected elements, the
overlap means the volume of the mesh does not match the physical space volume, raising concerns about accuracy.

We support two heuristics to detect these overlaps:

- detecting when the centroid of an element is inside another element
- detecting when a node of an element is inside another element

!alert note
These two checks are heuristics. They can have false negatives if elements overlap but in a way that the centroids
and nodes remain outside of other elements. This can happen for example with curved side elements overlapping on a side.

## Non-conformality check

Turn this check on with the [!param](/Mesh/MeshDiagnosticsGenerator/examine_non_conformality) parameter.

Non-conformality in MOOSE is generally only supported if it is arising from adaptive mesh refinement created
by the underlying mesh library. Additional information about the parent/child relationship between coarse and fine
elements is stored at the element level. However, numerous external meshes, especially arising from simulations
using adaptive mesh refinement, will present non-conformality. To support them, we usually resort to special numerical treatments,
such as mortar methods.

Non-conformality is detected by this diagnostic by looking for nodes that are near an element within a tolerance but
not part of this element's nodes.

!alert note
If the non-conformality is extremely minor, and the non-conformal nodes are very close to other nodes,
they can be merged together using a [MeshRepairGenerator.md].

### Non-conformality arising from Adaptive Mesh Refinement

Turn this check on with the [!param](/Mesh/MeshDiagnosticsGenerator/search_for_adaptivity_nonconformality) parameter.

In the special case of meshes where isotropic adaptive h-refinement was performed, the `MeshDiagnosticsGenerator` can detect
at the interface between coarse and fine elements that the fine elements could be combined into a coarse element.
The coarse element, if it were refined with MOOSE h-adaptivity, would be refined into the same fine elements as
originally present in the mesh.

This check will only detect mesh refinement for triangle and quadrilateral 2D elements and, tetrahedral and hexahedral 3D
elements. For tetrahedrals, because the refinement pattern depends on the selection of a diagonal inside the coarse element,
the check only considers the `tip` fine elements on the four vertex of the coarse element.

!alert note
This check is a heuristic. It will only detect adaptive *uniform* mesh refinement and it will only detect it
near interfaces between coarse and fine elements. If the entire mesh is refined the same number of times for each
element, it will not be detected.

!alert note
This check may report false positives on some immersed meshes and slit meshes because it will detect non-conformal nodes
and will try to examine if the elements next to these nodes can be grouped to form coarser elements.

## Elements and sides local Jacobian check

The local Jacobian is checked for positivity for every element and sides. Negative Jacobians are a common issue
with a poor, usually externally generated, mesh.

!alert note
A fifth-order Gauss quadrature is used for this check. If you intend to use another quadrature, for example a higher
order to integrate more accurately high order finite element variables,
feel free to modify the generator for your needs.

## Example input syntax: all diagnostics turned on

In this example, we examine a very basic mesh with all the diagnostics offered by the `MeshDiagnosticsGenerator`.
This simple mesh presents no issue.

!listing test/tests/meshgenerators/mesh_diagnostics_generator/all_at_once.i

!syntax parameters /Mesh/MeshDiagnosticsGenerator

!syntax inputs /Mesh/MeshDiagnosticsGenerator

!syntax children /Mesh/MeshDiagnosticsGenerator
