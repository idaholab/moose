# BinaryRecombinationBC

!syntax description /BCs/BinaryRecombinationBC

## Overview

This class implements a weak form corresponding to

\begin{equation}
\int_{\Omega} \psi_i K_r uv d\Omega
\end{equation}

where $K_r$ is the recombination coefficient specified with [!param](/BCs/BinaryRecombinationBC/Kr),
$u$ corresponds to [!param](/BCs/BinaryRecombinationBC/variable) and $v$ is a coupled variable specified by [!param](/BCs/BinaryRecombinationBC/v).
As the name states, this class is meant to model binary recombination
reactions occuring at a boundary.

!syntax parameters /BCs/BinaryRecombinationBC

!syntax inputs /BCs/BinaryRecombinationBC

!syntax children /BCs/BinaryRecombinationBC
