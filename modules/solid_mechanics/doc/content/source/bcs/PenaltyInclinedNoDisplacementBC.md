# PenaltyInclinedNoDisplacementBC

!syntax description /BCs/PenaltyInclinedNoDisplacementBC

## Description

`PenaltyInclinedNoDisplacementBC` is a `IntegratedBC` used for enforcing inclined boundary conditions $\mathbf{u}\cdot \mathbf{normal} = 0$ for mechanics problems. With a penalty method, the residual is given as
\begin{equation}
\mathcal{R}_i = \alpha(\mathbf{u}\cdot \mathbf{normal})\mathbf{normal}(\text{component})\psi_i
\end{equation}
where $\alpha$ is the penalty parameter and `component` corresponds to the direction in which to apply the residual. The normal directly comes from the surface normal defined in a mesh.
