# MultiAppPostprocessorTransfer

!syntax description /Transfers/MultiAppPostprocessorTransfer

## Siblings transfer behavior

This transfer supports sending data from a multiapp to a multiapp. There are two supported configurations:

- only one subapp in the source multiapp, the source postprocessor value is sent to all target multiapps.
- the same number of subapps in the source and target multiapp, the source postprocessor is matched to a target postprocessor based on a matching subapp index.

## Example Input File Syntax

The MultiAppPostprocessorTransfer allows for a Postprocessor value to be transfer between the
parent application and sub-application(s). For example, the input file snippet below
sets up a transfer of a Postprocessor value from the parent application to a Postprocessor on
each of the sub-applications.

!listing multiapp_postprocessor_transfer/parent.i block=Transfers

!syntax parameters /Transfers/MultiAppPostprocessorTransfer

!syntax inputs /Transfers/MultiAppPostprocessorTransfer

!syntax children /Transfers/MultiAppPostprocessorTransfer
