# LineMaterialRealSampler

!syntax description /VectorPostprocessors/LineMaterialRealSampler

## Overview

This class samples Real material properties for the integration points
in all elements that are intersected by a user-defined line.

The output to CSV is **by default** ordered as follows:

- rows are ordered by point sampled along the line

- columns are ordered by alphabetical order of the properties sampled. The distance along the sampled line and the x, y and z coordinates of the sampled points are added to the output as additional columns.

!alert note title=Vector names / CSV output column names
`LineMaterialSampler` declares a vector for each spatial coordinate, (`x`, `y`, `z`), of the sampled points,
the distance along the sampled line in a vector called `id`,
and a vector named after each material property sampled, containing the material property values at each point.

## Example Input File Syntax

In this example, the material property `matp` is being sampled along the segment between
'0.125 0.375 0.0' and '0.875 0.375 0.0'. The output is then sorted by the element ids along
this line.

!listing test/tests/vectorpostprocessors/line_material_sampler/line_material_real_sampler.i block=Materials VectorPostprocessors

!syntax parameters /VectorPostprocessors/LineMaterialRealSampler

!syntax inputs /VectorPostprocessors/LineMaterialRealSampler

!syntax children /VectorPostprocessors/LineMaterialRealSampler
