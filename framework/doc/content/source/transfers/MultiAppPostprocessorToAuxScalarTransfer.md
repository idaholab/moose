# MultiAppPostprocessorToAuxScalarTransfer

!syntax description /Transfers/MultiAppPostprocessorToAuxScalarTransfer

## Example Input File Syntax

The MultiAppPostprocessorToAuxScalarTransfer transfers a Postprocessor value to an scalar
AuxVariable. In the following example, a Postprocessor value from the parent application
is transferred to a scalar AuxVariable on each of the sub-applications.

!listing multiapp_postprocessor_to_scalar/parent.i block=Transfers

!syntax parameters /Transfers/MultiAppPostprocessorToAuxScalarTransfer

!syntax inputs /Transfers/MultiAppPostprocessorToAuxScalarTransfer

!syntax children /Transfers/MultiAppPostprocessorToAuxScalarTransfer
