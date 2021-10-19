# TimeIntegratedPostprocessor

!syntax description /Postprocessors/TimeIntegratedPostprocessor

## Example input syntax

In this example we integrate variable `u` over the spatial domain using a `ElementIntegralVariablePostprocessor`
and then integrate it over time using a `TimeIntegratedPostprocessor`.

!listing test/tests/postprocessors/element_integral_var_pps/pps_old_value.i block=Postprocessors

!syntax parameters /Postprocessors/TimeIntegratedPostprocessor

!syntax inputs /Postprocessors/TimeIntegratedPostprocessor

!syntax children /Postprocessors/TimeIntegratedPostprocessor
