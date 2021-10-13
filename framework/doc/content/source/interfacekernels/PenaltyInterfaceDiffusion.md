# PenaltyInterfaceDiffusion

## Overview

`PenaltyInterfaceDiffusion` is an interface condition that forces
the variable equivalence using a penalty. At the same time,
the flux equivalence is implicitly satisfied as long as no other
side (interface) residual objects are present. Mathematically, it is
written as follows:
\begin{equation}
\begin{aligned}
& D \frac{\partial u}{ \partial n} = P (u - u_{neighbor}), \\
& D_{neighbor} \frac{\partial u_{neighbor}}{ \partial n_{neighbor}} = - P (u - u_{neighbor}).
\end{aligned}
\end{equation}
where $D$ and $D_{neighbor}$ are material properties for the current element and
its neighbor, respectively, $n$ and $n_{neighbor}$ are the outward normal vectors
from the element and its neighbor (typically $n = - n_{neighbor})$.
$P$ is the the penalty supplied in the parameter [!param](/InterfaceKernels/PenaltyInterfaceDiffusion/penalty).

## Example Input File Syntax

!listing test/tests/interfacekernels/hybrid/interface.i start=[./penalty] end=[../] include-end=true

!syntax parameters /InterfaceKernels/PenaltyInterfaceDiffusion

!syntax inputs /InterfaceKernels/PenaltyInterfaceDiffusion

!syntax children /InterfaceKernels/PenaltyInterfaceDiffusion
