# InterfaceDiffusion

## Overview

That is a  kernel for establishing flux equivalence on an interface as follow:
\begin{equation}
D \frac{\partial u}{ \partial n} = D_{neighbor} \frac{\partial u_{neighbor}}{ \partial n}.
\end{equation}
Here $D$ and $D_{neighbor}$ are material properties for the current element and the neighbor, respectively.
$n$ is the outward normal vector from the current element. The kernel might not be used as
stand-alone interface conditions since it does not care about variable continuities. Instead, the kernel
could be combined with [MatchedValueBC](MatchedValueBC.md) or other strong interface conditions.

## Example Input File Syntax

!listing test/tests/interfacekernels/hybrid/interface.i start=[./diffusion] end=[../] include-end=true

!! Describe and include an example of how to use the InterfaceDiffusion object.

!syntax parameters /InterfaceKernels/InterfaceDiffusion

!syntax inputs /InterfaceKernels/InterfaceDiffusion

!syntax children /InterfaceKernels/InterfaceDiffusion
