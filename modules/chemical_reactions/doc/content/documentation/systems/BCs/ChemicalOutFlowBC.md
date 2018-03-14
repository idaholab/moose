# ChemicalOutFlowBC

!syntax description /BCs/ChemicalOutFlowBC

Integrated boundary condition to specify $\nabla u$ on the boundary, which arises
from integation by parts on the diffusion term
\begin{equation}
\nabla \cdot \left(\phi D \nabla C_j \right)
\end{equation}
where $\phi$ is porosity and $D$ is the diffusivity.

!syntax parameters /BCs/ChemicalOutFlowBC

!syntax inputs /BCs/ChemicalOutFlowBC

!syntax children /BCs/ChemicalOutFlowBC
