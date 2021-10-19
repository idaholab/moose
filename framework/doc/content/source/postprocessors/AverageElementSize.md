# AverageElementSize

!syntax description /Postprocessors/AverageElementSize

Other relevant metrics about the [mesh](syntax/Mesh/index.md) may be obtained locally using an [ElementQualityAux.md].

## Example input syntax

In this example, an advection problem is solved at multiple mesh refinement levels for a [Method of Manufactured Solution](python/mms.md optional=true) study. The size of the mesh is output to CSV using the `AverageElementSize` postprocessor.

!listing test/tests/fvkernels/mms/advective-outflow/advection.i block=Postprocessors

!syntax parameters /Postprocessors/AverageElementSize

!syntax inputs /Postprocessors/AverageElementSize

!syntax children /Postprocessors/AverageElementSize
