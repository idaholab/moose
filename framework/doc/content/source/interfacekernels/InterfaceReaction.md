# InterfaceReaction

## Description

Specie M transports between two domains (domain 0 and domain 1), at the interface consider the First-order reaction is taking place:

\begin{equation}
M(0)\xrightleftharpoons[k_b]{k_f}M(1)
\end{equation}

With the quasi-steady-state reaction rate (equal to flux)

\begin{equation}
Reaction Rate = k_f C_0 - k_b C_1
\end{equation}

where $k_f$ is the forward reaction coefficient, and $k_b$ is the backward reaction coefficient. `InterfaceReaction` object is used to impose this condition. Associated kernel is:

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

