# WorkBalance

## Short Description

!syntax description /VectorPostprocessors/WorkBalance

## Description

The idea here is to compute per-processor metrics to help in determining the quality of a partitioning.

Currently computes: number of local elements, nodes, dofs and partition sides.  The partition sides are the sides of elements that are on processor boundaries (also known as the "edge-cuts" in partitioner lingo).  Also computes the "surface area" of each partition (physically, how much processor boundary each partitioning has).

## Important Notes

Note that this VPP only computes the complete vector on processor 0.  The vectors this VPP computes may be very large and there is no need to have a copy of them on every processor.

!syntax parameters /VectorPostprocessors/WorkBalance

!syntax inputs /VectorPostprocessors/WorkBalance

!syntax children /VectorPostprocessors/WorkBalance

!bibtex bibliography
