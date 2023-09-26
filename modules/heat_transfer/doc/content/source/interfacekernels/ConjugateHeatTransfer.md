# ConjugateHeatTransfer

!syntax description /InterfaceKernels/ConjugateHeatTransfer

## Description

This `InterfaceKernel` models conjugate heat transfer between a solid and a fluid.
At the interface the condition:

\begin{equation}
  -k \vec{n} \cdot \nabla T_s = h (T_s - T_f)
\end{equation}

is imposed. In this equation $k$ is the solid thermal conductivity, $T_s$ is the solid
temperature, $h$ is the heat transfer coefficient, and $T_f$ is the fluid temperature.

The condition is applied directly in the fluid equation where $h (T_f - T_s)$ appears as
a boundary contribution. In the solid equation, integration of the heat conduction term
results in a $-k \vec{n} \cdot \nabla T_s$ integrated over the boundary. The above equality
is used to substitute the right hand side.

Note, in general the fluid energy equation does not need to be solved for temperature.
Therefore, the parameter `variable` may be tied to internal energy. For these cases, the
parameter `T_fluid` must be provided.

This class adopts the convection that the primary side is the fluid side and the secondary side
is the solid side. Therefore, `variable` expects the primary variable of the fluid energy
equation and `neighbor_var` expects the solid temperature. It is assumed that the solid energy
equation is solved for the solid temperature.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/conjugate_heat_transfer/conjugate_heat_transfer.i
 block=InterfaceKernels

!syntax parameters /InterfaceKernels/ConjugateHeatTransfer

!syntax inputs /InterfaceKernels/ConjugateHeatTransfer

!syntax children /InterfaceKernels/ConjugateHeatTransfer

!bibtex bibliography
