# BinaryRecombinationBC

!syntax description /BCs/BinaryRecombinationBC

## Overview

This class implements a weak form corresponding to

\begin{equation}
\int_{\Omega} \psi_i K_r uv d\Omega
\end{equation}

where $K_r$ is the recombination coefficient specified with the `Kr` parameter,
$u$ corresponds to `variable` and $v$ is a coupled variable specified by the `v`
parameter. As the name states, this class is meant to model binary recombination
reactions occuring at a boundary.

!syntax parameters /BCs/BinaryRecombinationBC

!syntax inputs /BCs/BinaryRecombinationBC

!syntax children /BCs/BinaryRecombinationBC
