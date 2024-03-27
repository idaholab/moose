# ExtraIDIntegralVectorPostprocessor

!syntax description /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor


## Overview

The `ExtraIDIntegralVectorPostprocessor` object is a vector postprocessor to integrate or average input variables or material properties based on multiple extra element integer IDs.
First, it finds unique combinations of extra IDs, and then it computes separate integral values over input variables and material properties for elements with these unique combinations.
For reactor applications, component-wise values such as pin-by-pin power distribution can be easily tallied using this object when the mesh contains the appropriate IDs.

The `ExtraIDIntegralVectorPostprocessor` object needs the following parameter:

- [!param](/VectorPostprocessors/ExtraIDIntegralVectorPostprocessor/id_name): list of extra IDs by which to separate integrals

The following parameters are optional, but at least one of them should be specified:

- [!param](/VectorPostprocessors/ExtraIDIntegralVectorPostprocessor/variable): variables that this VectorPostprocessor operates on.

- [!param](/VectorPostprocessors/ExtraIDIntegralVectorPostprocessor/mat_prop): material properties that this VectorPostprocessor operates on.

Additionally, a user can control whether volume-integrated values or volume-averaged values are computed by setting the following parameter:

- [!param](/VectorPostprocessors/ExtraIDIntegralVectorPostprocessor/average): whether or not to compute volume average.

!alert note title=Vector names / CSV output column names
`ExtraIDIntegralVectorPostprocessor` declares a vector for each variable or material property, named after the variable or material property, holding the values of the integrals computed.

## Example Syntax

!listing test/tests/vectorpostprocessors/extra_id_integral/extra_id_vpp.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor

!syntax inputs /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor

!syntax children /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor
