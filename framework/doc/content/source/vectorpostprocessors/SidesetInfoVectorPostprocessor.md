# SidesetInfoVectorPostprocessor

!syntax description /VectorPostprocessors/SidesetInfoVectorPostprocessor

## Description

`SidesetInfoVectorPostprocessor` assembles information from sidesets and prints them to
a csv file. Currently, it allows to obtain `area`, `centroid`, and the bounding box
via `min` and `max`. `min` contains the minimum x, y, z coordinates of the
sideset, while `max` contains the respective maximum values. Note that `centroid`
is not guaranteed to be a point contained in the sideset.

!alert note title=Vector names / CSV output column names
`SidesetInfoVectorPostprocessor` uses the following names for the vectors / CSV column headers: 
`Boundary IDs`, `centroid_x`, `centroid_y`, `min_x`, `min_y`, `max_x`, `max_y`, `area`.

!syntax parameters /VectorPostprocessors/SidesetInfoVectorPostprocessor

!syntax inputs /VectorPostprocessors/SidesetInfoVectorPostprocessor

!syntax children /VectorPostprocessors/SidesetInfoVectorPostprocessor
