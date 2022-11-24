# DissociationFluxBC

!syntax description /BCs/DissociationFluxBC

## Overview

This class implements a weak form corresponding to

\begin{equation}
- \int_{\Omega} \psi_i K_d v d\Omega
\end{equation}

where $K_d$ is a dissociation coefficient specified with [!param](/BCs/DissociationFluxBC/Kd) and
$v$ is the coupled variable specified with [!param](/BCs/DissociationFluxBC/v) that is
dissociating at the boundary to create the species corresponding to
[!param](/BCs/DissociationFluxBC/variable).

!syntax parameters /BCs/DissociationFluxBC

!syntax inputs /BCs/DissociationFluxBC

!syntax children /BCs/DissociationFluxBC
