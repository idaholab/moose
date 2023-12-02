# AverageSectionValueSampler

!syntax description /VectorPostprocessors/AverageSectionValueSampler

## Description

This vector postprocessor locates a number of nodes in a structural component and computes their average
variables along the $X$, $Y$, and $Z$ axes on multiple cross sections. This feature can be used to compute,
for example, average displacements.

The location of the nodes is given by a direction and an
associated length. This postprocessor is particularly indicated for structural components that deform in
a way analogous to beams, thereby freeing the user from performing additional postprocessing tasks on a
different software. One use case of this postprocessor is the definition of cross sections in ducts that
bend or bow (see [disp_cross]).

!media tensor_mechanics/disp_cross_section.png
    id=disp_cross
    caption=Undeformed and deformed configurations of a duct with a cross section defined by
    nodes in a regularly meshed geometry.
    style=display:block;margin-left:auto;margin-right:auto;width:60%

!alert note
This postprocessor should only be used for regularly meshed geometries as it relies on nodes having the
same weights on the cross section. This requirement must be considered by the user.

## Example Syntax

See below an input file excerpt which locates the cross section along the $Z$ direction at distances of 10 and 18.

!listing modules/tensor_mechanics/test/tests/cross_section_deflection/test_one_step.i block=VectorPostprocessors

Additionally, if multiple structural components share the same mesh block, one can set the input parameters
`reference_point` and `cross_section_maximum_radius` to point to the structural component's local frame
and the maximum in-cross-section-plane distance within. These parameters disambiguate which structural
component the nodes belong. An example of computing average section variables for multiple strucutral
components sharing the same mesh block is given below:

!listing modules/tensor_mechanics/test/tests/cross_section_deflection/test_one_step_two_ducts.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/AverageSectionValueSampler

!syntax inputs /VectorPostprocessors/AverageSectionValueSampler

!syntax children /VectorPostprocessors/AverageSectionValueSampler
