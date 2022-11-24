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

## Example Syntax

!listing test/tests/meshgenerators/advanced_extruder_generator/gen_extrude.i block=Mesh/extrude

!syntax parameters /Mesh/AdvancedExtruderGenerator

!syntax inputs /Mesh/AdvancedExtruderGenerator

!syntax children /Mesh/AdvancedExtruderGenerator
