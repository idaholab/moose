# MeshMetaDataReporter

A Reporter object for outputting mesh metadata in a human-readable form. By default, it executes only on initial.

This object is a specialization of [RestartableDataReporter.md], which you can see for a more detailed parameter description.

See [MeshMetaDataInterface.md] for more information on the use of mesh metadata.

## Example Input Syntax

The following input file snippet demonstrates the use of the `MeshMetaDataReporter` object.

!listing mesh_meta_data_reporter/mesh_meta_data_reporter.i block=Reporters/metadata

!syntax parameters /Reporters/MeshMetaDataReporter

!syntax inputs /Reporters/MeshMetaDataReporter

!syntax children /Reporters/MeshMetaDataReporter
