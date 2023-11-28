# SectionDisplacementAverage

!syntax description /VectorPostprocessors/SectionDisplacementAverage

## Description

This vector postprocessor locates a number of nodes in a structural component and computes their average
displacement along the $X$, $Y$, and $Z$ axes on multiple cross sections.
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


!syntax parameters /VectorPostprocessors/SectionDisplacementAverage

!syntax inputs /VectorPostprocessors/SectionDisplacementAverage

!syntax children /VectorPostprocessors/SectionDisplacementAverage
