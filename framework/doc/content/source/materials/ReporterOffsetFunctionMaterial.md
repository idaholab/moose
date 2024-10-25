# ReporterOffsetFunctionMaterial

!syntax description /Materials/ReporterOffsetFunctionMaterial

This can be used to create a material that is the sum of a function that is shifted by a set of points. This can be useful for creating a field containing multiple sources, see Figure??.


## Example Input File Syntax

In this example, `ReporterOffsetFunctionMaterial` is used to define a set of guassian heatsources. The value at a point is the sum of all the shifted functions.

!listing test/tests/materials/reporter_offset/reporter_offset_mat.i block=Materials/multiple_sources

!syntax parameters /Materials/ReporterOffsetFunctionMaterial

!syntax inputs /Materials/ReporterOffsetFunctionMaterial

!syntax children /Materials/ReporterOffsetFunctionMaterial
