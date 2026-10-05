# AdvancedExtruderGenerator

!syntax description /Mesh/AdvancedExtruderGenerator

## Overview

The `AdvancedExtruderGenerator` adds more customization options over the [MeshExtruderGenerator](MeshExtruderGenerator.md). This mesh generator is also capable of fixing inverted elements on-the-fly during extrusion.

## Multiple Elevations

`AdvancedExtruderGenerator` extrudes a lower-dimension mesh (1D or 2D) given by [!param](/Mesh/AdvancedExtruderGenerator/input) into a higher-dimension mesh (2D or 3D) in a direction defined by [!param](/Mesh/AdvancedExtruderGenerator/direction). The extruded mesh can have multiple elevations with variable extrusion (axial) lengths provided by [!param](/Mesh/AdvancedExtruderGenerator/heights). Each elevation can have separate subdomains, extra element extra integers, and boundaries defined. The number of axial elements in the different elevations can be provided through [!param](/Mesh/AdvancedExtruderGenerator/num_layers). Within each elevation, the axial element dimension can be biased using the corresponding growth factor value in [!param](/Mesh/AdvancedExtruderGenerator/biases).

## Subdomain ID Remapping

By default, the extruded higher-dimension elements retain the same subdomain ids as their original lower-dimension elements. `AdvancedExtruderGenerator` provides an option to remap subdomain ids for each elevation through [!param](/Mesh/AdvancedExtruderGenerator/subdomain_swaps), which is a double indexed array input parameter. Each elemental vector of [!param](/Mesh/AdvancedExtruderGenerator/subdomain_swaps) contains subdomain remapping information for a particular elevation, where the first elemental vector represents the first extruded elevation. The elemental vector contain pairs of subdomain ids: the first subdomain id is the `input` mesh subdomain id that needs to be remapped, and the second subdomain id the new subdomain id to be assigned.

## Extra Element Integer ID Remapping

Extra element integer ID remapping works in a similar manner as subdomain ID remapping. The extra element integers to be remapped must already exist in the `input` mesh and need to be specified in [!param](/Mesh/AdvancedExtruderGenerator/elem_integer_names_to_swap). Leveraging the recently updated [MOOSE input file syntax](https://mooseframework.inl.gov/application_usage/input_syntax.html) system, the remapping information of multiple extra element integers is provided as a triple-indexed array input parameter ([!param](/Mesh/AdvancedExtruderGenerator/elem_integers_swaps)). For each extra element integer, the syntax is similar to [!param](/Mesh/AdvancedExtruderGenerator/subdomain_swaps). The following input example shows the remapping of two extra element integers.

!listing test/tests/meshgenerators/advanced_extruder_generator/advanced_extruder_with_element_extra_integer_swap.i block=Mesh/extrude

## Boundary ID Remapping

Boundary ID remapping also works similarly to subdomain ID remapping. During extrusion, the lower-dimension boundaries are also converted into higher-dimension boundaries. A double indexed array input parameter, [!param](/Mesh/AdvancedExtruderGenerator/boundary_swaps), can be used to remap the boundary ids. Here, the boundary ids to be remapped must exist in the `input` mesh, otherwise, dedicated boundary defining mesh generators, such as [SideSetsBetweenSubdomainsGenerator](SideSetsBetweenSubdomainsGenerator.md) and [SideSetsAroundSubdomainGenerator](SideSetsAroundSubdomainGenerator.md), need to be used to define new boundary ids along different axial heights.

## Interface Boundaries

The other categories of the boundaries that can be defined are the interfaces between subdomains in different elevations, as well as the top/bottom surfaces of the subdomains. As each elevation interface (or top/bottom surface) is simply a duplicate of the `input` mesh, these interface (or top/bottom surface) boundaries correspond to the subdomains of the `input` mesh, which are referred to as `source_blocks`. Also, due to the nature of sidesets, they can be defined on either side of the elevation interface. Thus, both `upward` and `downward` boundaries can be defined. Here `upward` means the normal vector of the sideset has the "same-ish" direction as the [!param](/Mesh/AdvancedExtruderGenerator/direction) vector; `downward` means the normal vector of the sideset has the "opposite-ish" direction as the [!param](/Mesh/AdvancedExtruderGenerator/direction) vector.

## Extrusion along a line mesh / curve

Instead of a fixed direction, the mesh can be extruded following a line mesh. The extrusion is performed following the direction of the line mesh.
The direction is computed locally for every node as:

- the [!param](/Mesh/AdvancedExtruderGenerator/start_extrusion_direction) if specified at the first curve node, else the direction from the first to the second node
- the direction from the previous node to the next node (ignoring the current node) for every other curve node
- the [!param](/Mesh/AdvancedExtruderGenerator/end_extrusion_direction) if specified at the last curve node, else the direction from the last but one to the last node

!alert note
When extruding along an line mesh curve, specifying the biases, heights, number of layers and directions is not currently supported.

## Extrusion along node normals

Instead of a fixed [!param](/Mesh/AdvancedExtruderGenerator/direction), each node can be extruded along the
local surface normal by setting [!param](/Mesh/AdvancedExtruderGenerator/extrude_along_node_normals) to `true`.
This is useful to grow boundary layers off of a curved surface. The [!param](/Mesh/AdvancedExtruderGenerator/heights),
[!param](/Mesh/AdvancedExtruderGenerator/num_layers) and [!param](/Mesh/AdvancedExtruderGenerator/biases) parameters are
used as usual to control the layer thicknesses and grading; only the extrusion direction changes.

The direction at each node is computed as the average of the normals of all the elements connected to that node. Each
element normal is itself the average of its per-vertex normals, so the result reduces to the face normal for triangles and
planar quadrilaterals. The input surface mesh is expected to have consistently oriented elements. To flip the overall
growth direction (inward versus outward), negate the [!param](/Mesh/AdvancedExtruderGenerator/heights).

This option is only supported when extruding a 2D surface mesh into 3D, and is not currently implemented for distributed
meshes.

!listing test/tests/meshgenerators/advanced_extruder_generator/extrude_along_normals.i block=Mesh/extrude

## Radial growth during extrusion

By setting the [!param](/Mesh/AdvancedExtruderGenerator/end_radial_extent), the radial extent of the extruded mesh can be expanded or
diminished progressively throughout the extrusion process. The radial extent is defined as the maximum norm for all nodes of the extruded mesh
of the distance the node to the extrusion axis. For example, for a circle that is being extruded along an axis passing through its center,
the radial extent is equal to the radius of the circle.
The [!param](/Mesh/AdvancedExtruderGenerator/radial_growth_method) can be
set to a linear or cubic radial profile. The `cubic` option allows for specifying the derivative of the radial growth factor at
the beginning and end of the extrusion using the [!param](/Mesh/AdvancedExtruderGenerator/start_radial_growth_rate) and
[!param](/Mesh/AdvancedExtruderGenerator/end_radial_growth_rate) parameters respectively.

## Helicoidal extrusion

The user may elect to perform a helicoidal extrusion along an axis going through the (0, 0, 0) point aligned with
the extrusion [!param](/Mesh/AdvancedExtruderGenerator/direction) vector parameter. The user must then select a non-zero
[!param](/Mesh/AdvancedExtruderGenerator/twist_pitch) parameter.
After an extrusion distance of a pitch, a full rotation of the 2D shape being extruded will have been performed.
By default, the rotation is performed in a clockwise direction around the axis of extrusion. To change the direction of the rotation, please specify a negative [!param](/Mesh/AdvancedExtruderGenerator/twist_pitch).

## Example Syntax

!listing test/tests/meshgenerators/advanced_extruder_generator/gen_extrude.i block=Mesh/extrude

!syntax parameters /Mesh/AdvancedExtruderGenerator

!syntax inputs /Mesh/AdvancedExtruderGenerator

!syntax children /Mesh/AdvancedExtruderGenerator
