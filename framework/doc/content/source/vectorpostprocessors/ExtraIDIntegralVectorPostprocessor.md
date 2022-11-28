# ExtraIDIntegralVectorPostprocessor

!syntax description /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor


## Overview

The `ExtraIDIntegralVectorPostprocessor` object is a vector postprocessor to integrate input variables based on multiple extra element integer IDs.
First, it finds unique combinations of extra IDs, and then it computes separate integral values over input variables for elements with these unique combinations.
For reactor applications, component-wise values such as pin-by-pin power distribution can be easily tallied using this object when the mesh contains the appropriate IDs.

The `ExtraIDIntegralVectorPostprocessor` object needs the following parameters:

- [!param](/VectorPostprocessors/ExtraIDIntegralVectorPostprocessor/variable): variables that this VectorPostprocessor operates on.

- [!param](/VectorPostprocessors/ExtraIDIntegralVectorPostprocessor/id_name): list of extra IDs by which to separate integrals

!alert note title=Vector names / CSV output column names
`ExtraIDIntegralVectorPostprocessor` declares a vector for each variable, named after the variable,
holding the values of the integrals computed.

## Example Syntax

!listing test/tests/vectorpostprocessors/extra_id_integral/extra_id_vpp.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor

!syntax inputs /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor

!syntax children /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor
