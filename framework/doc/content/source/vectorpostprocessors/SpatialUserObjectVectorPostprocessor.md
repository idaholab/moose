# SpatialUserObjectVectorPostprocessor

!syntax description /VectorPostprocessors/SpatialUserObjectVectorPostprocessor

## Overview

This VectorPostprocessor evaluates a spatial user object at a set of points
and sets each postprocessor to the spatial value of the user object at each point.
This postprocessor may be used to convert spatial user objects such as
[NearestPointLayeredAverage](/userobject/NearestPointLayeredAverage.md) to a
postprocessor that can be transferred to another application with a
[MultiAppVectorPostprocessorTransfer](/transfers/MultiAppVectorPostprocessorTransfer.md).

The `use_points_from_uo` parameter can be used with user objects that define
the `spatialPoints()` interface to automatically get points in space at which the
user object obtains unique values.

## Example Input File Syntax

!listing test/tests/vectorpostprocessors/spatial_userobject_vector_postprocessor/spatial_userobject.i
  start=UserObjects
  end=Executioner

!syntax parameters /VectorPostprocessors/SpatialUserObjectVectorPostprocessor

!syntax inputs /VectorPostprocessors/SpatialUserObjectVectorPostprocessor

!syntax children /VectorPostprocessors/SpatialUserObjectVectorPostprocessor
