# InterfaceReaction

## Description

Specie M transports between two domains (domain 1 and domain 2), at the interface consider the following reaction is taking place:

\begin{equation}
M(1)\xrightleftharpoons[k_b]{k_f}M(2)
\end{equation}

With the first order reaction rate assuming a quasi-steady-state

\begin{equation}
Reaction Rate = \frac {\partial C_1} {\partial t} = k_f C_1 - k_b C_2 \approx 0
\end{equation}

where $C_1$ is the specie concentration in domain 1, $C_2$ is the specie concentration in domain 2, $k_f$ is the forward reaction coefficient, and $k_b$ is the backward reaction coefficient. `InterfaceReaction` object is used to impose this condition. Associated kernel is:

[/InterfaceReaction.C]

[/InterfaceReaction.h]

In addition, fluxes are matched from both domains, this could be achieved by  [`InterfaceDiffusion`](/InterfaceKernels/index.md). 

Both kernels at the interface work together to give full mathematical and physical meaning of the problem.

Two examples (steady-state and transient-state) are shown in the MOOSE test directory, 

[1d_interface/reaction_1D_steady.i]

[1d_interface/reaction_1D_transient.i]


## Example Input Syntax

!listing test/tests/interfacekernels/1d_interface/reaction_1D_steady.i start=[./interface_reaction] end=[../] include-end=true

!syntax parameters /InterfaceKernels/InterfaceReaction

!syntax inputs /InterfaceKernels/InterfaceReaction

!syntax children /InterfaceKernels/InterfaceReaction
