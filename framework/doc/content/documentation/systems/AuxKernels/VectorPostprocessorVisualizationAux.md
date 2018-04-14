# VectorPostprocessorVisualizationAux

## Short Description

!syntax description /AuxKernels/VectorPostprocessorVisualizationAux

## Description

This object is intended to let you view VectorPostprocessor vectors that are of lenght `num_procs` (meaning there is one value per MPI process).  This object will take those values and fill up an Auxiliary field with them so the values can be visualized.

## Important Notes

Note: the VectorPostprocessor must be syncing the vectors it's computing to all processors.  By default many just compute to processor 0 (because that's where output occurrs).

For instance: this is the case for [WorkBalance](WorkBalance.md).  By default it only syncs to processor 0, but it has a parameter (`sync_to_all_procs`) to tell it to create copies of the vectors on all processors.

!syntax parameters /AuxKernels/VectorPostprocessorVisualizationAux

!syntax inputs /AuxKernels/VectorPostprocessorVisualizationAux

!syntax children /AuxKernels/VectorPostprocessorVisualizationAux

!bibtex bibliography
