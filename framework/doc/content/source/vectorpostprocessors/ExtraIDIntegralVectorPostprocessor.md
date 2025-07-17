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

!listing test/tests/vectorpostprocessors/extra_id_integral/extra_id_vpp.i block=VectorPostprocessors

## Example use to output a field of the integral values

The `ExtraIDIntegralVectorPostprocessor` object can also be used as a functor for providing spatial values of the integrated or averaged values.
A [FunctorAux.md] could then be used to output the local values of this functor (the integrals) as an auxiliary *elemental* variable.
The variable has element-wise constant values. The value of the integral with each unique combination of extra ids is output to all the elements sharing that same combination.
When the `ExtraIDIntegralVectorPostprocessor` object is used as a functor, [!param](/VectorPostprocessors/ExtraIDIntegralVectorPostprocessor/spatial_value_name) must be set to indicate which variable or material property is to be used.

!alert note
When the `ExtraIDIntegralVectorPostprocessor` object is used as a functor, for example in [FunctorAux.md], [!param](/VectorPostprocessors/ExtraIDIntegralVectorPostprocessor/force_preaux) should set to true, otherwise the value of this object would likely be lagged in the functor evaluation because user objects defining functors are not put into the `pre_aux` group automatically by MOOSE.

!listing test/tests/vectorpostprocessors/extra_id_integral/functor_test.i start=VectorPostprocessors end=Executioner

## Example Syntax

!syntax parameters /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor

!syntax inputs /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor

!syntax children /VectorPostprocessors/ExtraIDIntegralVectorPostprocessor
