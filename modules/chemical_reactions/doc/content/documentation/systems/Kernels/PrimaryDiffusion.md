# PrimaryDiffusion

!syntax description /Kernels/PrimaryDiffusion

Diffusive flux of the $j^{\mathrm{th}}$ primary species. Implements the weak form of
\begin{equation}
\nabla \cdot \left(\phi D \nabla C_j \right)
\end{equation}
where $\phi$ is porosity and $D$ is the diffusivity.

!syntax parameters /Kernels/PrimaryDiffusion

!syntax inputs /Kernels/PrimaryDiffusion

!syntax children /Kernels/PrimaryDiffusion
