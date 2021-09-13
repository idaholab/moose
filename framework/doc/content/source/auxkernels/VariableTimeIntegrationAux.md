# VariableTimeIntegrationAux

!syntax description /AuxKernels/VariableTimeIntegrationAux

The integration over time is lead over every quadrature point.

## Example syntax

In this example, the `VariableTimeIntegrationAux` computes the time integral of variable
`u` with a first, second and third order scheme respectively.

!listing test/tests/auxkernels/time_integration/time_integration.i block=AuxKernels

!syntax parameters /AuxKernels/VariableTimeIntegrationAux

!syntax inputs /AuxKernels/VariableTimeIntegrationAux

!syntax children /AuxKernels/VariableTimeIntegrationAux
