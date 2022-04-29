# MultiAppPostprocessorNearestElementTransfer

!syntax description /Transfers/MultiAppPostprocessorNearestElementTransfer

## Description

This transfer object works just with `CONSTANT` `MONOMIAL` variables and is meant to be used mainly with `CentroidMultiapp`. Interpolating `FIRST` (or higher) order variables using the Multiapp postprocessors values centered in the multiapp position is not possible close to the variable domain edge and do not ensure the preservation of conserved quantities.

!alert note title=Partial domain Multiapp
In case the Multiapps are not defined in the whole variable domain (the multiapp blocks group is a subgroup of the variable blocks group), the elements in the blocks without a multiapp will get the value of the nearest Multiapp's postprocessor in the closest block in which multiapps are defined.

!syntax parameters /Transfers/MultiAppPostprocessorNearestElementTransfer

!syntax inputs /Transfers/MultiAppPostprocessorNearestElementTransfer

!syntax children /Transfers/MultiAppPostprocessorNearestElementTransfer

!bibtex bibliography
