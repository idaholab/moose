# MultiAppPostprocessorToAuxScalarTransfer

!syntax description /Transfers/MultiAppPostprocessorToAuxScalarTransfer

## Siblings transfer behavior

This transfer supports sending data from a MultiApp to a MultiApp if and only if the number of subapps
in the source MultiApp matches the number of subapps in the target MultiApp, and they are distributed
the same way on the parallel processes. Each source app is then matched to the target app with the same
subapp index.

## Example Input File Syntax

The MultiAppPostprocessorToAuxScalarTransfer transfers a Postprocessor value to an scalar
AuxVariable. In the following example, a Postprocessor value from the parent application
is transferred to a scalar AuxVariable on each of the sub-applications.

!listing multiapp_postprocessor_to_scalar/parent.i block=Transfers

!syntax parameters /Transfers/MultiAppPostprocessorToAuxScalarTransfer

!syntax inputs /Transfers/MultiAppPostprocessorToAuxScalarTransfer

!syntax children /Transfers/MultiAppPostprocessorToAuxScalarTransfer
