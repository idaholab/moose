# NodalMaxValue

!syntax description /Postprocessors/NodalMaxValue

!alert note
If the solution space is non convex over each element, the maximum of a variable may not be at a node.

## Example input syntax

In this example, `v` is the solution of diffusion problem. We examine the maximum value of
the variable over block 1.

!listing test/tests/postprocessors/nodal_max_value/block_nodal_pps_test.i block=Postprocessors

!syntax parameters /Postprocessors/NodalMaxValue

!syntax inputs /Postprocessors/NodalMaxValue

!syntax children /Postprocessors/NodalMaxValue
