# MultiAppVectorPostprocessorTransfer

MultiAppVectorPostprocessorTransfer transfers values from parent [VectorPostprocessors] to sub-app
[Postprocessors] or vice versa. The number of entries in the [VectorPostprocessors] data vector must
be equal to the number of sub-apps associated with the provided [MultiApps](/MultiApps/index.md). The transfer
sends/receives the n-th entry of the VectorPostprocessor to/from the n-th sub-app.

!syntax description /Transfers/MultiAppVectorPostprocessorTransfer

!alert note
For direct VectorPostprocessor-to-VectorPostprocessor transfers see [MultiAppReporterTransfer](MultiAppReporterTransfer.md#vector_transfer).

!syntax parameters /Transfers/MultiAppVectorPostprocessorTransfer

!syntax inputs /Transfers/MultiAppVectorPostprocessorTransfer

!syntax children /Transfers/MultiAppVectorPostprocessorTransfer
