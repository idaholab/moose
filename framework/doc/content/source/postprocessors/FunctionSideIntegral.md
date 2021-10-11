# FunctionSideIntegral

!syntax description /Postprocessors/FunctionSideIntegral

The sideset may be an internal or external boundary.
The function is evaluated at each quadrature point on the specified sideset. The default quadrature rule is used for integration.

## Example input syntax

In this example test, we compute a few function integrals on the boundaries of
the domain. Since the mesh is Cartesian, the integrals are known.

!listing test/tests/postprocessors/function_sideintegral/function_sideintegral.i block=Postprocessors

!syntax parameters /Postprocessors/FunctionSideIntegral

!syntax inputs /Postprocessors/FunctionSideIntegral

!syntax children /Postprocessors/FunctionSideIntegral
