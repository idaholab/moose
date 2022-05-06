# MultiAppVariableValueSamplePostprocessorTransfer

!syntax description /Transfers/MultiAppVariableValueSamplePostprocessorTransfer

## Description

This transfer supports data transfers between a variable on the main application and a postprocessor on the sub-applications.
The variable can be a component of an array variable or a standard field variable.

When transferring a variable to a postprocessor on the sub-applications of a MultiApp, the positions of sub-applications are used to sample the values of the variable and the sampled values are used to set the postprocessor values.

When transferring a postprocessor on the sub-applications of a MultiApp to a variable, the postprocessor value of a sub-application whose position is closest to an element is used to set the variable at the element.
Because of this, this transfer object works just with `CONSTANT` `MONOMIAL` variables and is meant to be used mainly with `CentroidMultiapp`.
Interpolating `FIRST` (or higher) order variables using the Multiapp postprocessors values centered in the multiapp position is not possible close to the variable domain edge and do not ensure the preservation of conserved quantities.
The target variable must be an auxiliary variable.

!alert note title=Partial domain Multiapp
In case the sub-applications are not defined in the whole variable domain (e.g. the CentroidMultiApp blocks group is a subgroup of the variable blocks group), the elements in the blocks without a multiapp will get the value of the nearest sub-application's postprocessor in the closest block in which sub-applications are defined.

## Example Input File Syntax

The following input file snippet demonstrates the use of the
MultiAppVariableValueSamplePostprocessorTransfer to transfer the value of a field variable
at the sub-application positions to a Postprocessor on each sub-application.

!listing centroid_multiapp/centroid_multiapp.i block=Transfers

!syntax parameters /Transfers/MultiAppVariableValueSamplePostprocessorTransfer

!syntax inputs /Transfers/MultiAppVariableValueSamplePostprocessorTransfer

!syntax children /Transfers/MultiAppVariableValueSamplePostprocessorTransfer
