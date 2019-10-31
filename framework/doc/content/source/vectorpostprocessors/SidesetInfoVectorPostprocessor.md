# SidesetInfoVectorPostprocessor

!syntax description /VectorPostprocessors/SidesetInfoVectorPostprocessor

## Description

SidesetInfoVectorPostprocessor assembles information from sidesets and prints them to
a csv file. Currently, it allows to obtain `area`, `centroid`, and the bounding box
via `min` and `max`. `min` contains the minimum x, y, z coordinates of the
sideset, while `max` contains the respective maximum values. Note that `centroid`
is not guaranteed to be a point contained in the sideset.

!syntax parameters /VectorPostprocessors/SidesetInfoVectorPostprocessor

!syntax inputs /VectorPostprocessors/SidesetInfoVectorPostprocessor

!syntax children /VectorPostprocessors/SidesetInfoVectorPostprocessor

!bibtex bibliography
