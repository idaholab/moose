# PenaltyInterfaceDiffusion

## Overview

`PenaltyInterfaceDiffusion` is a penalty-based interface condition that forces
the continuity of variables across an interface, and the flux equivalence,
as follows:
\begin{equation}
\begin{aligned}
& D \frac{\partial u}{ \partial n} = P (u - u_{neighbor}), \\
& D_{neighbor} \frac{\partial u_{neighbor}}{ \partial n_{neighbor}} = - P (u - u_{neighbor}).
\end{aligned}
\end{equation}
Here $D$ and $D_{neighbor}$ are material properties for the current elmetn and
its neighbor, respectively. $n$ and $n_{neighbor}$ are outward normal vectors
from the element and its neighbor, and normally, $n = - n_{neighbor}$.
$P$ is the penalty chosen by users.

## Example Input File Syntax

!listing test/tests/interfacekernels/hybrid/interface.i start=[./penalty] end=[../] include-end=true

!syntax parameters /InterfaceKernels/PenaltyInterfaceDiffusion

!syntax inputs /InterfaceKernels/PenaltyInterfaceDiffusion

!syntax children /InterfaceKernels/PenaltyInterfaceDiffusion
