# SpatialUserObjectVectorPostprocessor

!syntax description /VectorPostprocessors/SpatialUserObjectVectorPostprocessor

## Overview

This VectorPostprocessor evaluates a spatial user object at a set of points
and sets each postprocessor to the spatial value of the user object at each point.
This postprocessor may be used to convert spatial user objects such as
[NearestPointLayeredAverage](/userobjects/NearestPointLayeredAverage.md) to a
postprocessor that can be transferred to another application with a
[MultiAppVectorPostprocessorTransfer](/transfers/MultiAppVectorPostprocessorTransfer.md).

If neither [!param](/VectorPostprocessors/SpatialUserObjectVectorPostprocessor/points)
or [!param](/VectorPostprocessors/SpatialUserObjectVectorPostprocessor/points_file) are
provided, this object will attempt to get the spatial points directly from the user object.
These points will represent locations in space where the user object obtains a unique value.
To use this feature, the user object must define the `spatialPoints()` interface.

!alert note title=Vector names / CSV output column names
`SpatialUserObjectVectorPostprocessor` declares a vector with its own name. The full reporter name ends up being `<vpp_object_name>/<vpp_object_name>`.

## Example Input File Syntax

!listing test/tests/vectorpostprocessors/spatial_userobject_vector_postprocessor/spatial_userobject.i
  start=UserObjects
  end=Executioner

!syntax parameters /VectorPostprocessors/SpatialUserObjectVectorPostprocessor

!syntax inputs /VectorPostprocessors/SpatialUserObjectVectorPostprocessor

!syntax children /VectorPostprocessors/SpatialUserObjectVectorPostprocessor
