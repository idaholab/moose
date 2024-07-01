# ADMatInterfaceReaction

## Description

Specie M transports between two domains (domain 1 and domain 2), at the interface consider the following reaction is taking place:

\begin{equation}
M(1)\xrightleftharpoons[k_b]{k_f}M(2)
\end{equation}

With the first order reaction rate assuming a quasi-steady-state

\begin{equation}
\textrm{Reaction Rate} = \frac {\partial C_1} {\partial t} = k_f C_1 - k_b C_2 \approx 0
\end{equation}

where $C_1$ is the specie concentration in domain 1, $C_2$ is the specie concentration in domain 2, $k_f$ is the forward reaction coefficient, and $k_b$ is the backward reaction coefficient. `ADMatInterfaceReaction` object is used to impose this condition.

[InterfaceDiffusion.md] is also used in this case to control flux at the interface.

\begin{equation}
D_1 \frac {\partial C_1} {\partial n} = D_2 \frac {\partial C_2} {\partial n}
\end{equation}

However, the flux is not [well-defined](https://en.wikipedia.org/wiki/Well-defined_expression) across the interface. The `ADMatInterfaceReaction` interfacekernel applies a condition to constrain the potential discontinuity across the interface.

Both kernels at the interface work together to give full mathematical and physical meaning of the problem. Together, the implicit equations represented by `ADMatInterfaceReaction` and [InterfaceDiffusion.md] combine to provide the following relationship at the interface.

\begin{equation}
D_1 \frac {\partial C_1} {\partial n} = D_2 \frac {\partial C_2} {\partial n} + k_f C_1 - k_b C_2
\end{equation}

## Example Input Syntax

!listing test/tests/interfacekernels/1d_interface/ADMatreaction_1D_steady.i start=[interface_reaction] end=[] include-end=true

!listing test/tests/interfacekernels/1d_interface/ADMatreaction_1D_transient.i start=[interface_reaction] end=[] include-end=true

!syntax parameters /InterfaceKernels/InterfaceReaction

!syntax children /InterfaceKernels/ADMatInterfaceReaction
