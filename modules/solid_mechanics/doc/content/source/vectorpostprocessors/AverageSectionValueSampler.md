# AverageSectionValueSampler

!syntax description /VectorPostprocessors/AverageSectionValueSampler

## Description

This computes the average value of specified nodal variables at nodes located within cross-sectional planes
at locations along the axis of a mesh.  Those locations can either be defined by the user (through the [!param](/VectorPostprocessors/AverageSectionValueSampler/positions) parameter),
or automatically determined by the axial positions of the nodes in the mesh if [!param](/VectorPostprocessors/AverageSectionValueSampler/positions) is not specified.
For example, this is designed to work on bodies whose mesh discretization
is created by extrusion, so that sets of nodes that make up the mesh lie within planes regularly.

The location of the nodes is given by a direction and an
associated set of positions. This postprocessor is particularly indicated for structural components that deform in
a way analogous to beams, thereby freeing the user from performing additional postprocessing tasks on a
different software. One use case of this postprocessor is the definition of cross sections in ducts that
bend or bow (see [disp_cross]).

!media solid_mechanics/disp_cross_section.png
    id=disp_cross
    caption=Undeformed and deformed configurations of a duct with a cross section defined by
    nodes in a regularly meshed geometry.
    style=display:block;margin-left:auto;margin-right:auto;width:60%

!alert note
This postprocessor should only be used for regularly meshed geometries as it relies on nodes having the
same weights on the cross section. There is no check within the code for this condition, so the user must
ensure that the mesh is reasonably uniform.

Because this VectorPostprocessor is intended to be applied to geometries with extruded meshes, with layers
of nodes that all occupy the same axial position, it will generate an error if the numbers of nodes located
at the various axial positions vary. However, there are valid scenarios when this is not the case. For example,
if part of the extruded mesh is refined, the number of nodes per layer will differ along the axis of the
extruded section. The [!param](/VectorPostprocessors/AverageSectionValueSampler/require_equal_node_counts) parameter can be set to `false` in such situations to skip
that check. The number of nodes per axial position is reported in the ouputs as the `node_count` vector to allow the
user to ensure that each layer contains all expected nodes.

The optional [!param](/VectorPostprocessors/AverageSectionValueSampler/symmetry_plane) parameter is intended to be used when there is a symmetry plane passing through
the extruded section being evaluated. If there is volumetric expansion in the section and this is used to 
compute displacements, the results will be skewed because only the expansion on one side of the symmetry plane
is considered, when it should be balanced out by an expansion in the opposite direction on the other side of
that symmetry plane. To account for this, if [!param](/VectorPostprocessors/AverageSectionValueSampler/symmetry_plane) is specified, the set of variables is treated as a size-three vector, and the component of that vector in the direction of the specified symmetry plane is removed. The [!param](/VectorPostprocessors/AverageSectionValueSampler/symmetry_plane) parameter defines the normal to the symmetry plane, and when this option is used, a set of
three variables must be provided in [!param](/VectorPostprocessors/AverageSectionValueSampler/variables). These are assumed to be the x, y, and z components of a vector, in that order.

## Example Syntax

See below an input file excerpt which locates the cross section along the $Z$ direction at distances of 10 and 18.

!listing modules/solid_mechanics/test/tests/cross_section_deflection/test_one_step.i block=VectorPostprocessors

Additionally, if multiple structural components share the same mesh block, one can set the input parameters
[!param](/VectorPostprocessors/AverageSectionValueSampler/reference_point) and [!param](/VectorPostprocessors/AverageSectionValueSampler/cross_section_maximum_radius) to point to the structural component's local frame
and the maximum in-cross-section-plane distance within. These parameters disambiguate which structural
component the nodes belong. An example of computing average section variables for multiple strucutral
components sharing the same mesh block is given below:

!listing modules/solid_mechanics/test/tests/cross_section_deflection/test_one_step_two_ducts.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/AverageSectionValueSampler

!syntax inputs /VectorPostprocessors/AverageSectionValueSampler

!syntax children /VectorPostprocessors/AverageSectionValueSampler
