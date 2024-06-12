# RevolveGenerator

!syntax description /Mesh/RevolveGenerator

## Overview

This `RevolveGenerator` provides an alternative tool for increasing the dimensionality of a lower dimension mesh (1D or 2D) in addition to [MeshExtruderGenerator](MeshExtruderGenerator.md)/[AdvancedExtruderGenerator](AdvancedExtruderGenerator.md). Each element is converted
to one or more copies of its corresponding higher dimensional element along an open or closed specific circular curve.

The `RevolveGenerator` can provides similar customization options as in [AdvancedExtruderGenerator](AdvancedExtruderGenerator.md).

## Revolving Basics

`RevolveGenerator` revolves a lower-dimension mesh (1D or 2D) given by [!param](/Mesh/RevolveGenerator/input) into a higher-dimension mesh (2D or 3D) along an revolving axis defined by [!param](/Mesh/RevolveGenerator/axis_point) and [!param](/Mesh/RevolveGenerator/axis_direction). By default, the revolving can be performed along a full closed circular curve (i.e., 360 degrees) with one uniform azimuthal section. Optionally, the revolving curve can be a partial circular curve; and (or) multiple azimuthal sections can be defined. These options can be selected by specifying [!param](/Mesh/RevolveGenerator/revolving_angles). As long as the summation of the angles listed in [!param](/Mesh/RevolveGenerator/revolving_angles) is 360 degrees, a full closed circular revolving is performed. Otherwise, a partial revolving is conducted. For partial revolving or full revolving with multiple azimuthal sections, it can be conducted either clockwise or counter-clockwise, as controlled by [!param](/Mesh/RevolveGenerator/clockwise).

Each azimuthal sections can have separate subdomains, extra element extra integers, and boundaries defined. The number of azimuthal elements in the different sections can be provided through [!param](/Mesh/RevolveGenerator/nums_azimuthal_intervals).

## Subdomain ID Remapping

By default, the revolved higher-dimension elements retain the same subdomain ids as their original lower-dimension elements. `RevolveGenerator` provides an option to remap subdomain ids for each azimuthal section through [!param](/Mesh/RevolveGenerator/subdomain_swaps), which is a double indexed array input parameter. Each elemental vector of [!param](/Mesh/RevolveGenerator/subdomain_swaps) contains subdomain remapping information for a particular elevation, where the first elemental vector represents the first revolved azimuthal section. The elemental vector contain pairs of subdomain ids: the first subdomain id is the `input` mesh subdomain id that needs to be remapped, and the second subdomain id the new subdomain id to be assigned.

## Extra Element Integer ID Remapping

Extra element integer ID remapping works in a similar manner as subdomain ID remapping. The extra element integers to be remapped must already exist in the `input` mesh and need to be specified in [!param](/Mesh/RevolveGenerator/elem_integer_names_to_swap). The remapping information of multiple extra element integers is provided as a triple-indexed array input parameter ([!param](/Mesh/RevolveGenerator/elem_integers_swaps)). For each extra element integer, the syntax is similar to [!param](/Mesh/RevolveGenerator/subdomain_swaps). The following input example shows the remapping of two extra element integers.

!listing modules/reactor/test/tests/meshgenerators/revolve_generator/ei_swap.i block=Mesh/rg

## Boundary ID Remapping

Boundary ID remapping also works similarly to subdomain ID remapping. During revolving, the lower-dimension boundaries are also converted into higher-dimension boundaries. A double indexed array input parameter, [!param](/Mesh/RevolveGenerator/boundary_swaps), can be used to remap the boundary ids. Here, the boundary ids to be remapped must exist in the `input` mesh, otherwise, dedicated boundary defining mesh generators, such as [SideSetsBetweenSubdomainsGenerator](SideSetsBetweenSubdomainsGenerator.md) and [SideSetsAroundSubdomainGenerator](SideSetsAroundSubdomainGenerator.md), need to be used to define new boundary ids along different azimuthal sections.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/revolve_generator/revolve_2d.i block=Mesh/rg

!syntax parameters /Mesh/RevolveGenerator

!syntax inputs /Mesh/RevolveGenerator

!syntax children /Mesh/RevolveGenerator