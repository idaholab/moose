# MultiAppPostprocessorTransfer

!syntax description /Transfers/MultiAppPostprocessorTransfer

## Example Input File Syntax

The MultiAppPostprocessorTransfer allows for a Postprocessor value to be transfer between the
parent application and sub-application(s). For example, the input file snippet below
sets up a transfer of a Postprocessor value from the parent application to a Postprocessor on
each of the sub-applications.

!listing multiapp_postprocessor_transfer/parent.i block=Transfers

!syntax parameters /Transfers/MultiAppPostprocessorTransfer

!syntax inputs /Transfers/MultiAppPostprocessorTransfer

!syntax children /Transfers/MultiAppPostprocessorTransfer
