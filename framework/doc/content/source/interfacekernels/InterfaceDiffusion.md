# InterfaceDiffusion

## Overview

That is a kernel that establishes flux equivalence on an interface as follows:
\begin{equation}
D \frac{\partial u}{ \partial n} = D_{neighbor} \frac{\partial u_{neighbor}}{ \partial n},
\end{equation}
where $D$ and $D_{neighbor}$ are material properties for the current element and the neighbor, respectively, and
$n$ is the outward normal vector from the current element.

This kernel should not be used as a
stand-alone interface condition because it does not enforce variable continuity. Instead, consider
combining this kernel with [MatchedValueBC.md] or other strong interface conditions.

## Example Input File Syntax

!listing test/tests/interfacekernels/hybrid/interface.i start=[./diffusion] end=[../] include-end=true

!syntax parameters /InterfaceKernels/InterfaceDiffusion

!syntax inputs /InterfaceKernels/InterfaceDiffusion

!syntax children /InterfaceKernels/InterfaceDiffusion
