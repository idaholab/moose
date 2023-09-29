# FVConvectionCorrelationInterface

!syntax description /FVInterfaceKernels/FVConvectionCorrelationInterface

The surface convective flux to the fluid is then:
\begin{equation}
q_s = h_{correlation} (T_{solid} - T_{fluid})
\end{equation}
with $q_s$ the surface convective heat flux, $h_{correlation}$ the heat transfer coefficient
defined by the correlation as a material property and $T_{solid/fluid}$ the temperature of the
adjacent solid and fluid.

!alert note
This kernel supports interfaces between variables which belong to different nonlinear systems.
For instructions on how to set these cases up, visit the [FVInterfaceKernels syntax page](syntax/FVInterfaceKernels/index.md).

## Example input file syntax

In this example, a cold fluid is flowing next to a centrally-heated solid region. The heat diffuses
through the solid region, and convection at the interface transfers the heat to the fluid.

!listing modules/navier_stokes/test/tests/finite_volume/fviks/convection/convection_channel.i block=FVInterfaceKernels

!syntax parameters /FVInterfaceKernels/FVConvectionCorrelationInterface

!syntax inputs /FVInterfaceKernels/FVConvectionCorrelationInterface

!syntax children /FVInterfaceKernels/FVConvectionCorrelationInterface
