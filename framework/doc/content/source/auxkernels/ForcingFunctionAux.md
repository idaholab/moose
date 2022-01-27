# ForcingFunctionAux

!syntax description /AuxKernels/ForcingFunctionAux

This `AuxKernel` adds a forcing function to the value of an `AuxVariable` from the previous
time step. For each time step $\delta t$, the value of the `AuxVariable` is computed as
\begin{equation}
V(t + \Delta t) = V(t) + f \Delta t
\end{equation}
where $V$ is the `AuxVariable` and $f$ is the forcing function. $f$ is a MOOSE `function`
that is specified as an input parameter.

## Example Input File Syntax

In this example, value of a postprocessor is supplied to the forcing function f used by the `ForcingFunctionAux` `AuxKernel`, which increments the `AuxVariable` T.

!listing /test/tests/auxkernels/forcing_function_aux/forcing_function_aux.i block=AuxKernels

!syntax parameters /AuxKernels/ForcingFunctionAux

!syntax inputs /AuxKernels/ForcingFunctionAux

!syntax children /AuxKernels/ForcingFunctionAux
