# NodalMaxValueId

!syntax description /Postprocessors/NodalMaxValueId

This postprocessor performs a reduction across all ranks to compute the maximum, then returns the id of the node with the global maximum.

## Example input syntax

In this input file we obtain both the maximum value of variable `u` using a [NodalExtremeValue.md] and the location where it is reached using a `NodalMaxValueId`.

!listing test/tests/postprocessors/nodal_extreme_value/nodal_extreme_pps_test.i block=Postprocessors

!syntax parameters /Postprocessors/NodalMaxValueId

!syntax inputs /Postprocessors/NodalMaxValueId

!syntax children /Postprocessors/NodalMaxValueId
