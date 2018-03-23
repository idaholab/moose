# Vector Postprocessor Transfer

!syntax description /Transfers/VectorPostprocessorTransfer

## Description

`VectorPostprocessorTransfer` transfers vectorpostprocessor data from the sub applications to auxvariables in the master application. The VectorPostprocessor data in the `subapp_component` direction is extracted along with the data corresponding to the list of variable vectors. This data is linearly interpolated and mapped to auxvariables in the master model. The linear interpolation is performed only along one coordinate direction specified by `master_component`. The `positions` file specifies the location in the master model in directions other than the `master_component`. The `positions` file must contain only two columns and must contain same number of rows as the number of sub applications. 

## Example Input File Syntax

!listing modules/combined/test/tests/beam_eigenstrain_transfer/master.i block=Transfers/fromsub

!syntax parameters /Transfers/VectorPostprocessorTransfer

!syntax inputs /Transfers/VectorPostprocessorTransfer

!syntax children /Transfers/VectorPostprocessorTransfer
